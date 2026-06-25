# Day53 Stack Pivot Hard-1

Goal: no `fake_stack` global variable and no direct write to `.bss`.

You must:
1. find the overflow offset,
2. find useful ROP gadgets,
3. choose a writable runtime address from the RW LOAD segment,
4. build a small first-stage ROP chain that calls `read(0, writable_addr, size)`,
5. return to a pivot gadget,
6. send the second-stage fake stack into the writable address.

Binary is stripped. PIE is off, canary is off, NX is on.
