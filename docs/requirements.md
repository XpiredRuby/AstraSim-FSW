# Requirements

| ID | Requirement | Verification Method | Status |
|---|---|---|---|
| REQ-001 | Flight software shall start in BOOT mode | Run output / future unit test | Implemented |
| REQ-002 | Flight software shall transition to NOMINAL after initialization | Run output / future unit test | Implemented |
| REQ-003 | CPU overload shall transition system to DEGRADED_PAYLOAD | Fault test | Implemented as simulated fault |
| REQ-004 | Critical sensor failure shall transition system to SAFE mode | Future HIL/SIL test | Planned |
| REQ-005 | Flight software shall send telemetry to laptop | Future UDP test | Planned |
| REQ-006 | Flight software shall receive commands from laptop | Future UDP command test | Planned |
| REQ-007 | Corrupted commands shall be rejected | Future command parser test | Planned |
| REQ-008 | Requirement checker shall output PASS/FAIL results | Future scenario test | Planned |
