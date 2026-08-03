Day87 — Ghidra 2: strings / xrefs / callers

Target: day87_challenge
Architecture: x86-64 Linux ELF, stripped PIE

Rules
- Do not brute-force the accepted input.
- Start from visible strings, follow xrefs, then move upward through callers.
- Rename functions only after recording evidence.

Deliverable: day87_rev.md
- protection / file information
- important imports
- useful strings and their xrefs
- decoy string(s) and why they are not on the normal path
- recovered main flow
- function rename table with evidence
- accepted alias and phrase conditions
- one objdump or GDB cross-check
- one failure-path reproduction
