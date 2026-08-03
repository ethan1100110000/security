Day88 — Ghidra 3: rename / type recovery

Target: day88_challenge
Architecture: x86-64 Linux ELF, stripped PIE

Goal
- Recover the shared context structure from repeated field offsets.
- Rename functions and variables only after recording evidence.
- Apply a custom structure type so the decompiler shows fields instead of raw pointer arithmetic.
- Recover the accepted input and explain the validation flow.

Required evidence
- Context structure layout: offset, size, proposed field name/type.
- Before/after decompiler comparison after applying the structure.
- Function rename table with evidence.
- One GDB or objdump cross-check.
- One failure-path reproduction.

CS
- Cases where Ghidra decompiler output should not be trusted without Listing/GDB verification.

Deliverable
- day88_rev.md
- screenshots or command log showing type recovery and cross-check
