# ASTRA-OS Engineering Decisions

This file records decisions that materially affect interfaces, verification, or project identity. Decisions are append-only; superseded decisions remain visible with a link to the replacement.

## ADR-001 — Preserve the existing repository and working behavior

**Status:** Accepted  
**Decision:** Evolve `XpiredRuby/AstraSim-FSW` in place. Preserve history, current demonstrations, packet behavior, and validated scenarios unless a requirement-controlled change is necessary.  
**Reason:** The predecessor is a working engineering baseline with evidence. Rebuilding would discard traceability and create avoidable regression risk.  
**Consequence:** Refactors require regression evidence and cannot silently replace existing interfaces.

## ADR-002 — Retain C++17 during baseline hardening

**Status:** Accepted  
**Decision:** Keep the current core on C++17 while adding deterministic behavior and assurance infrastructure. Reconsider C++20 only when a specific feature provides measurable value.  
**Reason:** A language-version migration would increase change surface before the baseline is fully characterized.  
**Consequence:** C++20 is not an acceptance criterion for the first assurance release.

## ADR-003 — Preserve existing mode and fault numeric values

**Status:** Accepted  
**Decision:** Existing enum values are treated as interface values. New modes or faults shall be appended or introduced through an explicitly versioned interface rather than renumbering current values.  
**Reason:** Commands, telemetry, Python tools, scenarios, and historical evidence depend on the current representation.  
**Consequence:** STANDBY and TEST cannot be inserted by renumbering the current mode enum.

## ADR-004 — Enforce ground replay checks at the application boundary

**Status:** Accepted  
**Decision:** Apply duplicate and replay sequence checks in `FlightSoftwareApp` after packet decoding and before `CommandProcessor`.  
**Reason:** The packet decoder should remain a pure byte validator, while `CommandProcessor` must also process trusted internal fault commands that do not belong to the ground sequence stream.  
**Consequence:** Internal faults bypass the ground sequence guard by design. Their origin must remain explicit in future event telemetry.

## ADR-005 — Use unsigned half-range serial-number arithmetic

**Status:** Accepted  
**Decision:** A candidate 32-bit ground sequence is newer when the unsigned forward distance is nonzero and less than `2^31`.  
**Reason:** This permits deterministic wrap from `UINT32_MAX` to zero without signed-overflow behavior.  
**Consequence:** A jump of exactly `2^31` is rejected as ambiguous. Resynchronization requires a future explicit protocol operation.

## ADR-006 — Consume sequences for semantically rejected commands

**Status:** Accepted  
**Decision:** Once a packet is structurally valid and reaches the ground boundary, its sequence is consumed even if its command argument or transition is rejected.  
**Reason:** Reusing a sequence with altered semantics would weaken replay resistance and make acknowledgement behavior ambiguous.  
**Consequence:** Ground software must issue a new sequence after correcting a rejected command.

## ADR-007 — Keep the scheduler pure and callback-free

**Status:** Accepted  
**Decision:** `RateGroupScheduler` emits releases and tracks completion/deadline state but does not invoke application callbacks or read a system clock.  
**Reason:** Separating decision logic from execution makes schedules deterministic, portable, and independently testable.  
**Consequence:** The future cyclic executive must dispatch releases and report completion explicitly.

## ADR-008 — Treat CI evidence as generated, provenance-bound output

**Status:** Accepted  
**Decision:** Full verification and coverage outputs are generated in CI and uploaded as artifacts with commit, toolchain, host, command, and input-hash metadata.  
**Reason:** Committing large or frequently changing reports obscures source history and can preserve stale evidence.  
**Consequence:** Only compact stable evidence or summaries should be committed; reproducible bulk output belongs in CI artifacts.

## ADR-009 — Use a controlled defect before claiming test effectiveness

**Status:** Accepted  
**Decision:** The assurance pipeline must demonstrate that the command tests fail when CRC rejection is deliberately disabled in an isolated copy.  
**Reason:** Line coverage alone does not prove that assertions detect incorrect behavior.  
**Consequence:** Mutation survival fails the assurance job and blocks a verified disposition for the linked requirement.

## ADR-010 — Defer the governed AI assistant

**Status:** Accepted  
**Decision:** Do not implement or market the AI assurance assistant as a primary feature until core requirements, regression tests, static and dynamic analysis, timing evidence, and traceability are stable.  
**Reason:** An assistant could distract from the flight-software and assurance foundation and must not become a source of verification truth.  
**Consequence:** Early releases remain valuable without AI functionality.
