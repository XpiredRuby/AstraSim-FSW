# ASTRA-OS Risks and Blockers

Risk ratings are qualitative until the project defines a formal likelihood and consequence scale. A risk remains open until objective closure evidence is linked.

| ID | Risk or blocker | Impact | Current mitigation | Status |
|---|---|---|---|---|
| RISK-001 | The current execution environment cannot clone GitHub through its terminal DNS path. | Local builds cannot be independently run in the assistant sandbox. | Use authenticated GitHub changes and pull-request CI as the execution authority; archive logs and manifests. | Open |
| RISK-002 | Branch-protection settings have not been independently inspected. | Changes could be merged without required checks. | Keep the ASTRA-OS work on a draft pull request and do not merge until checks and review are complete. | Open |
| RISK-003 | Existing Raspberry Pi evidence predates the ASTRA-OS assurance branch. | HIL claims may be mistaken for current-branch execution. | Label Pi evidence Historical and require a new manifest for future HIL runs. | Controlled |
| RISK-004 | CRC provides corruption detection but not command authentication. | A network peer could construct a valid malicious command packet. | State the boundary explicitly; plan a separate authorization layer and adversarial tests. | Open |
| RISK-005 | Command timestamps are carried but not freshness-checked. | A packet with a newer sequence but unacceptable mission time could be processed. | Sequence replay guard is implemented; define mission-time tolerance and resynchronization protocol next. | Open |
| RISK-006 | Ground sequence resynchronization is not defined. | Ground restart or state loss could prevent valid commands or invite unsafe manual resets. | Preserve half-range behavior; add an explicit authorized resynchronization operation rather than implicit reset. | Open |
| RISK-007 | The new scheduler is not yet integrated into the production app loop. | Scheduler evidence does not yet prove end-to-end task timing behavior. | Keep scheduler requirements marked Implemented rather than Verified; add integration and timing scenarios. | Open |
| RISK-008 | Deadline ticks are abstract and not yet correlated to measured execution time. | Tick-level tests could be overinterpreted as real-time evidence. | Require host and Pi timing reports with exact tick period, clock source, load, and provenance. | Open |
| RISK-009 | Current FDIR policy is distributed across health, watchdog, command, and mode logic. | Simultaneous faults and priority behavior may be inconsistent. | Introduce an explicit FDIR policy component after preserving current regressions. | Open |
| RISK-010 | Existing fault codes do not all have detection, persistence, recovery, and verification evidence. | The project could claim ten FDIR cases based only on enum count. | Count only independently exercised fault cases; maintain a fault disposition matrix. | Open |
| RISK-011 | Aggregate coverage may hide weak modules. | A high overall percentage could mask untested safety-significant decisions. | Report per-module coverage and justified exclusions before publishing numerical claims. | Open |
| RISK-012 | clang-tidy configuration may differ across runner versions. | New runner images could introduce non-deterministic static-analysis failures. | Pin runner/tool versions where practical and preserve tool version in the manifest. | Open |
| RISK-013 | Sanitizer execution is host-only. | Host-clean behavior does not prove Raspberry Pi behavior or timing. | Treat sanitizers as dynamic software evidence, not HIL evidence; repeat applicable tests on target later. | Controlled |
| RISK-014 | The controlled mutation currently covers one CRC decision. | A single killed mutation does not establish broad test effectiveness. | Expand to a reviewed mutation set across mode guards, replay checks, health thresholds, and watchdog decisions. | Open |
| RISK-015 | Current requirement checking is Markdown- and scenario-centric. | Unit tests, source components, revisions, and changed-impact status are not fully checked. | Move to the canonical requirements file and verification matrix; extend the checker incrementally. | Open |
| RISK-016 | Existing reports in the repository may be stale relative to current source. | Readers may mistake old evidence for current results. | Generate CI artifacts per commit and include source hashes; mark preserved reports by provenance status. | Open |
| RISK-017 | Global generated-file patterns previously ignored all CSV files. | Required traceability CSV files could be omitted from normal Git workflows. | Scope ignores to generated report directories; commit the canonical verification matrix. | Mitigated |
| RISK-018 | The future AI assistant could overstate evidence or invoke unsafe tools. | Incorrect verification dispositions or unauthorized actions. | Defer implementation; enforce read-only retrieval, allow-listed tools, human approval, audit logs, and frozen adversarial tests. | Deferred |

## Human or physical blockers

No human action is currently required for software development. Human assistance will become necessary for:

- a new Raspberry Pi HIL execution requiring target access;
- physical power cycling or network reconfiguration;
- license activation for unavailable proprietary tools;
- destructive repository or visibility changes;
- authoritative mission choices that materially change the project identity.
