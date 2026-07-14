# ASTRA-OS Ten-Case FDIR Campaign

Result: PASS

Build directory: `build-pi`
Cases: `10`
Passed: `10`
Failed: `0`

The campaign injects each supported fault through the UDP command/telemetry scenario boundary. It verifies the typed acknowledgement, active fault indication, and resulting operational mode. It does not prove coverage of every physical detection mechanism.

| Fault | Expected mode | Result | Scenario report |
|---|---|---|---|
| `SENSOR_TIMEOUT` | `SAFE` | PASS | `reports/scenario_fdir_sensor_timeout_output.txt` |
| `SENSOR_INVALID_DATA` | `SAFE` | PASS | `reports/scenario_fdir_sensor_invalid_data_output.txt` |
| `PAYLOAD_HEARTBEAT_TIMEOUT` | `DEGRADED_PAYLOAD` | PASS | `reports/scenario_fdir_payload_heartbeat_timeout_output.txt` |
| `CPU_OVERLOAD` | `DEGRADED_PAYLOAD` | PASS | `reports/scenario_fdir_cpu_overload_output.txt` |
| `MEMORY_OVERLOAD` | `SAFE` | PASS | `reports/scenario_fdir_memory_overload_output.txt` |
| `TELEMETRY_SOCKET_FAILURE` | `SAFE` | PASS | `reports/scenario_fdir_telemetry_socket_failure_output.txt` |
| `COMMAND_BAD_CRC` | `NOMINAL` | PASS | `reports/scenario_fdir_command_bad_crc_output.txt` |
| `COMMAND_UNKNOWN_ID` | `NOMINAL` | PASS | `reports/scenario_fdir_command_unknown_id_output.txt` |
| `COMMAND_TIMEOUT` | `SAFE` | PASS | `reports/scenario_fdir_command_timeout_output.txt` |
| `WATCHDOG_DEADLINE_MISS` | `SAFE` | PASS | `reports/scenario_fdir_watchdog_deadline_miss_output.txt` |
