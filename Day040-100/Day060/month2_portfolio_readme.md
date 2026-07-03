# Month 2 Portfolio Summary

## 1. Scope

Day41~Day59 focused on advanced pwn fundamentals:
- FSB leaks
- Canary bypass
- PIE base calculation
- ret2libc
- stack pivot
- heap/fake-stack pivot
- shellcode
- failure-case debugging

## 2. Core Techniques

| Technique | Used In | Key Point |
|---|---|---|
| FSB leak | Day41, Day42, Day44, Day50, Day59 | Leak canary / PIE / stack / heap |
| Canary bypass | Day41, Day42, Day44, Day50, Day59 | Preserve exact canary before saved RBP |
| PIE bypass | Day43, Day44, Day47, Day50, Day59 | leak - known offset = PIE base |
| ret2libc | Day46, Day47 | Leak libc function, compute libc base |
| Stack pivot | Day59 | Move RSP to fake stack |
| ret2csu | Day59 | Control rdi/rsi/rdx and indirect call |
| Shellcode | Day48 | NX OFF + stack execution |

## 3. Representative Labs

| Day | Topic | Result | Notes |
|---|---|---|---|
| Day41 | FSB leak lab | Completed | Canary + PIE leak validation |
| Day42 | Canary ret2win | Completed | FSB leak + BOF |
| Day44 | Canary + PIE | Completed | Combined mitigation bypass |
| Day46 | External ret2libc | Completed | GOT leak + system("/bin/sh") |
| Day48 | Shellcode | Completed | NX OFF shellcode execution |
| Day50 | Mini exam | Completed | Stripped PIE + canary |
| Day59 | Mini exam 2 | Completed with note | P2 required binary patch due read-size issue |

## 4. Verification Checklist

- [ ] Canary low byte is `00`
- [ ] PIE base is page-aligned
- [ ] PIE leak belongs to executable mapping
- [ ] GOT/libc leak parsed without newline
- [ ] ROP stack alignment checked
- [ ] Pivot target memory is writable
- [ ] Fake stack layout verified with `x/gx`
- [ ] Final exploit reproduced from clean run

## 5. Failure Cases Learned
- FSB leak index can be misclassified if the leaked value is not validated against gdb or memory mappings.
- Canary leak must be checked for the null low byte and must be used in the exact stack position.
- PIE leak is only valid when `leak - offset` gives a page-aligned base.
- `puts`/`printf` leaks may include newline bytes; libc leaks should be parsed with `.strip()` and fixed-width slicing.
- ROP chains can fail due to stack alignment before calling libc functions such as `system`.
- A pivot target must point to writable memory that actually contains the fake stack.
- `call [r12 + rbx*8]` in ret2csu requires a pointer slot, not the function address directly.
- Input size limits must be checked before assuming a BOF reaches saved RIP.
## 6. CS Review
### ABI / Stack Alignment

- On x86-64 System V ABI, function call arguments are passed through `rdi`, `rsi`, `rdx`, `rcx`, `r8`, and `r9`.
- Before calling libc functions through ROP, stack alignment must be checked.
- If `system()` or another libc call crashes around `movaps`, add one `ret` gadget before the call.
- For ret2csu:
  - `r13` becomes `edi`
  - `r14` becomes `rsi`
  - `r15` becomes `rdx`
  - `call [r12 + rbx*8]` calls the function pointer stored at that address.
- For stack pivot:
  - `leave; ret` uses saved `rbp` as the next stack.
  - `xchg rax, rsp; ret` pivots directly to the address stored in `rax`.
## 7. Next Month Plan
- Move from pwn fundamentals to harder integrated labs.
- Strengthen reversing basics: function recovery, control-flow analysis, and patch diff reading.
- Start fuzzing workflow: harness, crash triage, minimization, and root-cause analysis.
- Convert selected labs into portfolio-quality write-ups.
- Keep exploit verification strict:
  - reproduce from clean run
  - record binary protections
  - explain leak source
  - explain final control-flow hijack


## 8. Reproduction Verification

Selected labs for reproduction:

| Day | Target | Command | Result |
|---|---|---|---|
| Day44 | Canary + PIE combined | `python3 exploit.py` | Verified |
| Day46 | External ret2libc | `python3 exploit.py` | Verified |
| Day50 | Mini exam | `python3 exploit.py` | Verified |
