# Test Plan

## Current Tests

1. Build the C++ flight software.
2. Run the executable.
3. Confirm BOOT mode appears.
4. Confirm BOOT to NOMINAL transition.
5. Confirm CPU overload fault transitions to DEGRADED_PAYLOAD.

## Manual Test Command

```bash
mkdir -p build
cd build
cmake ..
make -j4
./astra_fsw
```

## Expected Output

```text
AstraSim-FSW starting...
Mode: BOOT
Mode transition: BOOT -> NOMINAL
Heartbeat 1 | Mode: NOMINAL
Heartbeat 2 | Mode: NOMINAL
Heartbeat 3 | Mode: NOMINAL
Heartbeat 4 | Mode: NOMINAL
Heartbeat 5 | Mode: NOMINAL
Injecting simulated CPU overload fault...
Mode transition: NOMINAL -> DEGRADED_PAYLOAD | Reason: CPU_OVERLOAD
AstraSim-FSW shutdown.
```

## Future Tests

- UDP telemetry test
- Command parser test
- Watchdog timeout test
- Sensor fault test
- Payload fault test
- Monte Carlo scenario test
