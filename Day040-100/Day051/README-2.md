# Day51 harder variant: FSB byte write, hidden target

Rules:
- No source.
- Target address is not printed.
- Binary is stripped.
- Find the target byte address and required byte value from disassembly.
- Then use FSB write to pass the gate.

Suggested workflow:
1. checksec
2. run the binary
3. identify the vulnerable printf
4. find the global byte read before the gate comparison
5. identify the compare immediate value
6. find FSB offset
7. overwrite one byte with `%hhn` or `fmtstr_payload(..., write_size="byte")`
