# Live Telemetry Demo

This demo sends AstraSim-FSW binary telemetry over UDP.

## Terminal 1: Start Receiver

From the project root:

```bash
python3 tools/telemetry_receiver.py --port 5005
```

Expected startup message:

```text
Listening for AstraSim-FSW telemetry on UDP 0.0.0.0:5005
```

## Terminal 2: Send Telemetry

From the project root:

```bash
./build/astra_fsw_telemetry_demo 127.0.0.1 5005
```

## Expected Behavior

The receiver should print decoded telemetry packets like:

```text
seq=1 t_ms=... mode=BOOT fault=NONE cpu=19.00% mem=41.50% hb=1
seq=2 t_ms=... mode=NOMINAL fault=NONE cpu=20.00% mem=43.00% hb=2
...
seq=8 t_ms=... mode=DEGRADED_PAYLOAD fault=CPU_OVERLOAD cpu=91.00% mem=52.00% hb=8
```

## Purpose

This demonstrates:

- C++ binary telemetry packet serialization
- CRC-protected packet validation
- UDP telemetry transmission
- Python ground-side telemetry decoding
- Mode and fault state reporting
