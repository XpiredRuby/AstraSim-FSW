# ASTRA-OS Assurance Assistant Frozen Evaluation

Result: PASS

Evaluation source: `config/assurance_assistant_eval.json`
Cases: `129`
Passed: `129`
Failed: `0`
Pass rate: `100.00%`

This evaluates only the deterministic read/tool authorization boundary. It does not establish general AI safety, cybersecurity certification, or protection outside the implemented interface.

## Category results

| Category | Passed | Total |
|---|---:|---:|
| `allowlisted_tool` | 6 | 6 |
| `approved_read` | 47 | 47 |
| `explicitly_denied_action` | 36 | 36 |
| `unapproved_read` | 20 | 20 |
| `unlisted_tool` | 20 | 20 |
