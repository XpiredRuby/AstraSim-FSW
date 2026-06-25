#!/usr/bin/env bash
set -euo pipefail

PORT="${1:-5005}"
REPORT_FILE="reports/live_telemetry_demo_output.txt"
RECEIVER_LOG="$(mktemp)"
SENDER_LOG="$(mktemp)"

cleanup() {
  rm -f "$RECEIVER_LOG" "$SENDER_LOG"
}

trap cleanup EXIT

mkdir -p reports

if [ ! -x "./build/astra_fsw_telemetry_demo" ]; then
  echo "ERROR: ./build/astra_fsw_telemetry_demo does not exist."
  echo "Run: bash ci/run_local_tests.sh"
  exit 1
fi

echo "Starting telemetry receiver on UDP port ${PORT}..."
timeout 8s python3 -u tools/telemetry_receiver.py --port "$PORT" > "$RECEIVER_LOG" 2>&1 &
RECEIVER_PID=$!

sleep 1

echo "Running telemetry sender..."
./build/astra_fsw_telemetry_demo 127.0.0.1 "$PORT" > "$SENDER_LOG" 2>&1

wait "$RECEIVER_PID" || true

RECEIVED_COUNT="$(grep -c "seq=" "$RECEIVER_LOG" || true)"

{
  echo "# Live Telemetry Demo Output"
  echo
  echo "Generated: $(date -u +"%Y-%m-%dT%H:%M:%SZ")"
  echo
  echo "## Summary"
  echo
  echo "- UDP packets received: ${RECEIVED_COUNT}"
  echo "- Expected packets: 12"
  echo "- Expected fault transition: NOMINAL -> DEGRADED_PAYLOAD at sequence 8"
  echo
  echo "## Sender Output"
  echo
  echo '```text'
  cat "$SENDER_LOG"
  echo '```'
  echo
  echo "## Receiver Output"
  echo
  echo '```text'
  cat "$RECEIVER_LOG"
  echo '```'
} > "$REPORT_FILE"

if [ "$RECEIVED_COUNT" -ne 12 ]; then
  echo "ERROR: Expected 12 received packets, got ${RECEIVED_COUNT}."
  echo "See ${REPORT_FILE}"
  exit 1
fi

if ! grep -q "seq=8 .*mode=DEGRADED_PAYLOAD .*fault=CPU_OVERLOAD" "$RECEIVER_LOG"; then
  echo "ERROR: Did not find expected seq=8 fault transition in receiver output."
  echo "See ${REPORT_FILE}"
  exit 1
fi

echo "Live telemetry demo capture passed."
echo "Report written to ${REPORT_FILE}"
