#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${REPO_ROOT}/build"
PACKAGE_ROOT="${REPO_ROOT}/dist/astrasim-fsw-pi"
PACKAGE_ARCHIVE="${REPO_ROOT}/dist/astrasim-fsw-pi.tar.gz"
REPORT="${REPO_ROOT}/reports/pi_deployment_package_report.md"
PACKAGE_ARCHIVE_RELATIVE="dist/astrasim-fsw-pi.tar.gz"
REPORT_RELATIVE="reports/pi_deployment_package_report.md"

cd "${REPO_ROOT}"

mkdir -p "${BUILD_DIR}"
cmake -S . -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}" --target astra_fsw astra_fsw_command_telemetry_demo -j

rm -rf "${PACKAGE_ROOT}"
mkdir -p "${PACKAGE_ROOT}/bin" \
         "${PACKAGE_ROOT}/tools" \
         "${PACKAGE_ROOT}/scenarios" \
         "${PACKAGE_ROOT}/docs" \
         "${PACKAGE_ROOT}/reports"

cp "${BUILD_DIR}/astra_fsw" "${PACKAGE_ROOT}/bin/"
cp "${BUILD_DIR}/astra_fsw_command_telemetry_demo" "${PACKAGE_ROOT}/bin/"

cp tools/send_command.py "${PACKAGE_ROOT}/tools/"
cp tools/telemetry_receiver.py "${PACKAGE_ROOT}/tools/"
cp tools/run_scenario.py "${PACKAGE_ROOT}/tools/"
cp tools/run_hil_smoke_test.py "${PACKAGE_ROOT}/tools/"
cp tools/check_requirements.py "${PACKAGE_ROOT}/tools/"

cp scenarios/*.yaml "${PACKAGE_ROOT}/scenarios/"
cp docs/pi_deployment.md "${PACKAGE_ROOT}/docs/" 2>/dev/null || true
cp docs/hil_smoke_test.md "${PACKAGE_ROOT}/docs/" 2>/dev/null || true

cat > "${PACKAGE_ROOT}/README_PI.md" <<'MD'
# AstraSim-FSW Raspberry Pi Deployment Package

This package contains the flight software binaries and Python tools needed to run command/telemetry verification on a Raspberry Pi target.

## Target smoke command

```bash
./bin/astra_fsw_command_telemetry_demo 6000 127.0.0.1 5005 20
```

From another terminal on the same target, commands can be injected with:

```bash
python3 tools/send_command.py --port 6000 --sequence 1 --command SET_MODE --argument NOMINAL
```

Telemetry can be observed with:

```bash
python3 tools/telemetry_receiver.py --port 5005
```
MD

tar -czf "${PACKAGE_ARCHIVE}" -C "${REPO_ROOT}/dist" astrasim-fsw-pi

ARCHIVE_SIZE="$(du -h "${PACKAGE_ARCHIVE}" | awk '{print $1}')"
ARCHIVE_SHA="$(sha256sum "${PACKAGE_ARCHIVE}" | awk '{print $1}')"

mkdir -p "${REPO_ROOT}/reports"
cat > "${REPORT}" <<MD
# Raspberry Pi Deployment Package Report

Result: PASS

## Package

\`\`\`text
${PACKAGE_ARCHIVE_RELATIVE}
\`\`\`

## Included target binaries

- \`bin/astra_fsw\`
- \`bin/astra_fsw_command_telemetry_demo\`

## Included verification tools

- \`tools/send_command.py\`
- \`tools/telemetry_receiver.py\`
- \`tools/run_scenario.py\`
- \`tools/run_hil_smoke_test.py\`
- \`tools/check_requirements.py\`

## Package metadata

- Size: \`${ARCHIVE_SIZE}\`
- SHA256: \`${ARCHIVE_SHA}\`

## Notes

This report verifies that a Raspberry Pi deployment package can be generated from the repository. Running the package on physical Raspberry Pi hardware should produce a separate target-run evidence report.
MD

echo "Package written to ${PACKAGE_ARCHIVE_RELATIVE}"
echo "Report written to ${REPORT_RELATIVE}"
