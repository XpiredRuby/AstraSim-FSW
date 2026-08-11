# Contributing to ASTRA-OS

ASTRA-OS is an educational spacecraft-style flight-software and assurance project. Contributions are welcome when they make the implementation, verification, documentation, or claim boundaries more precise.

## Before opening a pull request

1. Keep claims bounded to evidence that exists in the repository.
2. Do not rewrite frozen release evidence to match a newer implementation. Historical artifacts describe the baseline that produced them.
3. Add or update tests when behavior changes.
4. Keep command/telemetry interfaces, requirements, and verification allocations synchronized.
5. Avoid weakening gates to make a change pass. Fix the implementation, test, or documented assumption instead.

## Local verification

At minimum, run:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DASTRA_WARNINGS_AS_ERRORS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
python3 -m unittest discover -s tools/tests -p 'test_*.py'
python3 tools/check_protocol_conformance.py
python3 tools/check_requirements.py
```

For changes to fault-management, command, telemetry, configuration, scheduling, or assurance behavior, also run the relevant deterministic scenario/fault campaign and the managed assurance workflow where practical.

## Pull-request expectations

A useful PR explains:

- the problem being solved;
- the behavior or evidence that changes;
- which requirements/interfaces are affected;
- which checks were run;
- any limitation that remains.

Generated evidence should be regenerated only when the source/configuration change legitimately requires it. Do not edit measured or frozen evidence simply to make a claim look current.

## Scope language

Preferred wording distinguishes among:

- portable software behavior;
- native target execution;
- software-in-the-loop or software-driven test campaigns;
- actual hardware-in-the-loop integration;
- qualification/certification.

ASTRA-OS does not claim certification, spacecraft flight qualification, hard-real-time/WCET guarantees, or representative spacecraft hardware integration.
