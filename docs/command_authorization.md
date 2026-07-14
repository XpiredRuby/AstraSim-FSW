# Ground-Command Authorization Policy

## Purpose

`CommandAuthorizer` applies an explicit execution policy after packet decoding and ground-command freshness/replay checks but before `CommandProcessor` may mutate flight-software state.

The policy can independently allow or deny:

- `NOOP`;
- `SET_MODE`;
- `INJECT_FAULT`;
- `REQUEST_TELEMETRY`;
- `CLEAR_FAULT`;
- entry into `TEST` mode.

The default configuration permits all existing commands and therefore preserves prior behavior. A denied command returns `REJECTED_UNAUTHORIZED`, does not change mode or fault state, is recorded in event telemetry, and consumes its already accepted sequence number.

## Processing order

```text
UDP packet decode and CRC validation
    -> timestamp and sequence guard
    -> command authorization policy
    -> semantic command processing
    -> mode or fault state mutation
    -> telemetry acknowledgement and event record
```

## Integrity versus authorization

CRC-16 detects accidental or injected packet corruption under the tested model. It does not identify a sender and is not a cryptographic authenticator. `CommandAuthorizer` is also not sender authentication; it is a configurable command-execution policy.

A production secure-command design would require mission-approved key management identity authentication anti-replay state operational roles and secure update procedures. Those are outside the present portfolio system.

## Evidence

- `command_authorizer_tests`
- `flight_software_app_tests`
- `config/assurance_assistant_policy.json` for a separate ground-tool permission example
- `config/protocol_manifest.json` for the typed `REJECTED_UNAUTHORIZED` acknowledgement
