#!/usr/bin/env bash
set -euo pipefail

rm -rf build
mkdir -p build
cd build
cmake ..
make -j"$(nproc)"
ctest --output-on-failure
./astra_fsw
