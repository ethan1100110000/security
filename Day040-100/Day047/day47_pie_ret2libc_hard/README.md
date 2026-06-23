# Day47 Local Challenge: PIE leak + libc leak + ret2libc

## Goal
Exploit `day47_pie_ret2libc_hard` with this exact flow:

1. Find a PIE/code pointer leak yourself.
2. Compute PIE base.
3. Set `elf.address`.
4. Build stage1 ROP to leak a libc address from GOT and return to `main`.
5. Compute libc base.
6. Build stage2 ret2libc shell.
7. Verify both PIE base and libc base with `vmmap`.

## Expected protections

```text
Canary: No canary
NX: enabled
PIE: enabled
RELRO: Partial RELRO
```

## Rules

- Do not count it as solved until both bases match `vmmap`.
- Do not use source code first unless you are stuck. Treat the binary as the challenge.
- Completion proof: show `id` from shell.

## Hints

- There are two inputs per run.
- The first bug can disclose stack values.
- The second bug is enough for ROP.
- The binary intentionally imports libc functions through PLT/GOT.
- If `system("/bin/sh")` is unstable on your libc/alignment, an `execve` ret2libc chain is also acceptable.
