# Problem 2 - hard PIE + canary + csu + xchg pivot

Run:

```bash
./day57_p2_hard
```

Expected study points:

- There is a format-string leak stage before the overflow stage.
- Use the leak stage to recover:
  - stack canary
  - PIE code pointer
- Compute PIE base, then adjust gadget/function/GOT offsets.
- The program prints a heap address. Use it as a fake stack location.
- Preserve the canary in the final stack overflow payload.
- Do not use `leave; ret` as the main pivot. Train `pop rax; ret` + `xchg rax, rsp; ret`.
- Use ret2csu staged indirect call through a writable function-pointer slot.

This problem intentionally combines: FSB leak, PIE base, canary preservation, xchg pivot, and ret2csu argument passing.
