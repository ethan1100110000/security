# Problem 1 - easy csu + xchg pivot

Run:

```bash
./day57_p1_easy
```

Expected study points:

- The printed `pivot here` address is a writable heap address.
- First input is written to that heap address.
- Second input is a small stack overflow.
- Use `pop rax; ret` and `xchg rax, rsp; ret` to move `rsp` to the heap fake stack.
- Use ret2csu twice:
  1. call `read(0, slot, 8)` to write a function pointer to a writable slot.
  2. call `[slot]` with the required three arguments.

This is not a heap exploitation problem. The heap is only used as a writable fake stack.
