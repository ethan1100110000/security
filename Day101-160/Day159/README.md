# FINAL Day159 — W07 3/5: constrained ROP / ABI / control-flow evidence

**Status: NOT STARTED.** Local *authorized educational CTF ELF* created for this Day. The original C code and intended gadget order are intentionally not disclosed. User must analyze the binary without copying a ready-made chain. Target proof is **local printed marker**, not a shell or outside system.

## First setup (WSL, glibc >= 2.34)
```bash
cd /mnt/d/security-roadmap
git pull --ff-only
cd Day101-160/Day159
bash unpack.sh
bash record.sh
```
After `record.sh`, the *new subshell* records the entire terminal session in `day159_session.log`. At the very end: `exit`. No repeating the whole experiment just to create evidence.

Compiled stripped PIE ELF `day159_stack.elf` SHA256:
`3512168bf96fd11b8337dc37e22b20257374eabeda5ab95ba612d0578a81452e`.
Maximum required GLIBC symbol version 2.34 (verified from `readelf --version-info` on build output). Stripped x86-64; Canary OFF, NX ON, PIE ON, Full RELRO. Exact input conditions, vulnerable stack slot, control-flow gate, ABI requirements and gadget addresses are for student discovery.

## Main: controlled CTF ROP construction
1. **Initial evidence / hypothesis**: `file`, `checksec`, `readelf -lW`, program run with benign input. Predict what the printed address might represent; verify in disassembly instead of assuming a leak is useful.
2. **Static reconstruction**: From stripped `objdump`/Ghidra identify the user-controlled input path, size validation (including forbidden bytes), memory copy, return point, successful controlled target, arguments, and potential registers/gadgets. Report uncertainties, not fabricated C declarations.
3. **Dynamic first-pass**: From locally controlled crash and GDB independently locate overwrite distance and instructions consuming the payload. Separate *actual saved return address overwrite* from memory corruption or early input rejection.
4. **Chain design**: Account for PIE/ASLR (same-process leak), SysV AMD64 argument registers, RSP%16 at final function entry, and prohibited bytes. Build local input/reproduction script yourself. Demonstrate controlled success in this educational ELF; if blocked, classify exact failed stage (parse / byte filter / displacement / mapping / gadget / ABI / stack alignment / branch check).
5. **Controlled failures**: At least one paired failure and success differing by one condition, with observed output and explanation. User must record environment, predictions, actual evidence, assumptions, remaining uncertainty, and own commit SHA. Do not claim success from teacher-run tests.

## CS: SysV ABI
- Distinguish caller-saved from callee-saved registers; inspect real call boundary or reconstruct saving/restoring RBX/RBP/R12–R15 as required.
- Confirm 16-byte alignment **before call** and RSP%16 = 8 **at normal function entry**, and how `pop` / `ret` alter those states.

## Review
- Day043 and Day050: use a *new* ROP condition; first independently predict the number of 8-byte stack slots consumed, then contrast with original Git write-ups. Do not repeat the same quiz.
- Due 2026-11-25: Day157 D+2 (PLT/GOT/lazy-binding inference) and Day155 D+7 (PIE/ASLR/offset evidence); apply one compact changed-condition review, noting where hints were needed.

## Evidence gate
A passing day requires main + CS + security review + due spaced retrieval + Git user-committed logs/script/write-up, with **predictions, raw outputs, independent evidence, and at least one changed-condition retest**. Teacher upload of this ELF/README does not count as student evidence; Day159 remains incomplete until student results are verified.
