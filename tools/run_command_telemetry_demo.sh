#!/usr/bin/env bash
set -euo pipefail

COMMAND_PORT="${1:-6000}"
TELEMETRY_PORT="${2:-5005}"
LOOP_COUNT="${3:-18}"

REPORT_FILE="reports/command_telemetry_demo_output.txt"
RECEIVER_LOG="$(mktemp)"
SERVER_LOG="$(mktemp)"
COMMAND_LOG="$(mktemp)"

cleanup() {
  rm -f "$RECEIVER_LOG" "$SERVER_LOG" "$COMMAND_LOG"
}

trap cleanup EXIT

mkdir -p reports

if [ ! -x "./build/astra_fsw_command_telemetry_demo" ]; then
  echo "ERROR: ./build/astra_fsw_command_telemetry_demo does not exist."
  echo "Run: bash ci/run_local_tests.sh"
  exit 1
fi

echo "Starting telemetry receiver on UDP port ${TELEMETRY_PORT}..."
timeout 14s python3 -u tools/telemetry_receiver.py --port "$TELEMETRY_PORT" > "$RECEIVER_LOG" 2>&1 &
RECEIVER_PID=$!

sleep 1

echo "Starting command/telemetry demo server on UDP command port ${COMMAND_PORT}..."
./build/astra_fsw_command_telemetry_demo "$COMMAND_PORT" 127.0.0.1 "$TELEMETRY_PORT" "$LOOP_COUNT" > "$SERVER_LOG" 2>&1 &
SERVER_PID=$!

sleep 1

echo "Sending SET_MODE NOMINAL command..."
python3 tools/send_command.py \
  --host 127.0.0.1 \
  --port "$COMMAND_PORT" \
  --sequence 1 \
  --command SET_MODE \
  --argument NOMINAL >> "$COMMAND_LOG" 2>&1

sleep 1

echo "Sending INJECT_FAULT CPU_OVERLOAD command..."
python3 tools/send_command.py \
  --host 127.0.0.1 \
  --port "$COMMAND_PORT" \
  --sequence 2 \
  --command INJECT_FAULT \
  --argument CPU_OVERLOAD >> "$COMMAND_LOG" 2>&1

sleep 1

echo "Sending CLEAR_FAULT command..."
python3 tools/send_command.py \
  --host 127.0.0.1 \
  --port "$COMMAND_PORT" \
  --sequence 3 \
  --command CLEAR_FAULT >> "$COMMAND_LOG" 2>&1

wait "$SERVER_PID"
wait "$RECEIVER_PID" || true

RECEIVED_COUNT="$(grep -c "seq=" "$RECEIVER_LOG" || true)"

{
  echo "# Command / Telemetry Demo Output"
  echo
  echo "Generated: $(date -u +"%Y-%m-%dT%H:%M:%SZ")"
  echo
  echo "## Summary"
  echo
  echo "- Command port: ${COMMAND_PORT}"
  echo "- Telemetry port: ${TELEMETRY_PORT}"
  echo "- Telemetry packets received: ${RECEIVED_COUNT}"
  echo "- Expected command sequence:"
  echo "  - seq=1 SET_MODE NOMINAL"
  echo "  - seq=2 INJECT_FAULT CPU_OVERLOAD"
  echo "  - seq=3 CLEAR_FAULT"
  echo
  echo "## Command Sender Output"
  echo
  echo '```text'
  cat "$COMMAND_LOG"
  echo '```'
  echo
  echo "## C++ Command / Telemetry Server Output"
  echo
  echo '```text'
  cat "$SERVER_LOG"
  echo '```'
  echo
  echo "## Python Telemetry Receiver Output"
  echo
  echo '```text'
  cat "$RECEIVER_LOG"
  echo '```'
} > "$REPORT_FILE"

if ! grep -q "RX command seq=1 id=SET_MODE status=ACCEPTED mode=NOMINAL fault=NONE" "$SERVER_LOG"; then
  echo "ERROR: SET_MODE NOMINAL command was not accepted."
  echo "See ${REPORT_FILE}"
  exit 1
fi

if ! grep -q "RX command seq=2 id=INJECT_FAULT status=ACCEPTED mode=DEGRADED_PAYLOAD fault=CPU_OVERLOAD" "$SERVER_LOG"; then
  echo "ERROR: INJECT_FAULT CPU_OVERLOAD command was not accepted."
  echo "See ${REPORT_FILE}"
  exit 1
fi

if ! grep -q "RX command seq=3 id=CLEAR_FAULT status=ACCEPTED mode=DEGRADED_PAYLOAD fault=NONE" "$SERVER_LOG"; then
  echo "ERROR: CLEAR_FAULT command was not accepted."
  echo "See ${REPORT_FILE}"
  exit 1
fi

if ! grep -q "mode=DEGRADED_PAYLOAD fault=CPU_OVERLOAD" "$RECEIVER_LOG"; then
  echo "ERROR: Telemetry receiver did not observe DEGRADED_PAYLOAD with CPU_OVERLOAD."
  echo "See ${REPORT_FILE}"
  exit 1
fi

if [ "$RECEIVED_COUNT" -lt 10 ]; then
  echo "ERROR: Expected at least 10 telemetry packets, got ${RECEIVED_COUNT}."
  echo "See ${REPORT_FILE}"
  exit 1
fi

echo "Command/telemetry demo capture passed."
echo "Report written to ${REPORT_FILE}"
