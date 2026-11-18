# Day155 / FINAL W06 3/4 — PIE × ASLR relocation measurements (in progress)

**Authoritative plan:** FINAL XLSX `01_일일124` Day155, 2026-11-18.
Main: **PIE ON/OFF × ASLR ON/OFF address-offset measurement (10 independent runs per viable condition)**; distinguish relocatable ELF image from system-wide ASLR mappings. Explain **without-leak attack uncertainty** with observed numbers and a plot or table. Do not infer full address entropy from only 10 trials.
CS: Distinguish PIE (ELF position independence/relocations) from ASLR (runtime address layout) and perform an **unhinted offset calculation**.
Review: Day040~050 stack/ROP/ABI, past Git notes first recall → source cross-check → changed-condition counterexample. Day153 2026-11-16 → D+2 memory gate on 2026-11-18; answer without notes first.

## Source / artifacts
- `addr_probe.c`: harmless local address probe. Prints function (main/helper), stack local, actual libc `puts` symbol via `dlsym`, and `main - helper`. **Printing addresses is a controlled measurement (an intentional address disclosure); the blind no-leak scenario must be discussed separately.**
- `build.sh`: creates two versions from the **same source**, `addr_no_pie` and `addr_pie`.
- `measure.py`: repeatedly launches two binaries and records 10 samples per file into CSV. OFF mode uses `setarch -R` to request no ASLR for only that process; if it is denied, records the limitation rather than inventing data.

## Preparation
```bash
cd /mnt/d/security-roadmap
git pull --ff-only
cd Day101-160/Day155
bash build.sh
cat /proc/sys/kernel/randomize_va_space
```
`readelf -hW` from build script should show no PIE ET_EXEC vs PIE ET_DYN. This is a **static property**, not proof that ASLR is currently enabled.

### Stage A — before execution
Read `addr_probe.c`, `build.sh`. Predict independently:
1. With ASLR ON, what changes across runs for main, stack and libc_puts in PIE-OFF vs PIE-ON?
2. Will `main_minus_helper` vary across process executions of the **same binary**?
3. Does "PIE disabled" imply libc and stack don't randomize? Why?
Provide the prediction before looking at 10-run statistics.

### Stage B — collect observations
```bash
python3 measure.py --aslr on --runs 10
```
The output `measure_aslr-on.csv` contains **10 separate program runs for each binary**. Inspect summary/CSV. Record raw sample count, unique addresses, stable offsets, compile flags, tool versions.

Then test OS-permitted ASLR-OFF conditions:
```bash
python3 measure.py --aslr off --runs 10
```
**Important:** In some WSL/container setups, `setarch ... -R` is blocked (`Operation not permitted`). Do **not** use global `/proc/sys/kernel/randomize_va_space` sysctl changes for this assignment. If blocked, record exactly what failed; optional local GDB test:
```gdb -q ./addr_pie
(gdb) set disable-randomization on
(gdb) run
```
If GDB also reports `Error disabling address space randomization`, mark ASLR-OFF untested with permission evidence. Don't mislabel runs.

### Stage C — cause / offset / no-leak uncertainty
- Use at least two evidence types: e.g. raw CSV + `readelf -hW` + `objdump -d -M intel` / maps.
- For each variant, compare how many distinct `main`, `stack`, `libc_puts`, and `main_minus_helper` values appear.
- For one PIE runtime sample, compare `main` runtime address with `main` ELF symbol offset (`nm -n addr_pie | grep ' main$'`). Infer image base and why it normally changes with ASLR.
- Quantify **within this 10-run sample** how many distinct page-aligned main bases are observed. This is **sample diversity**, not true ASLR entropy or guaranteed exploit probability.
- With **no runtime pointer disclosure**, explain why knowing a fixed `main→helper` offset alone does not tell you the absolute target address under PIE+ASLR. Make a small observed-address table/plot. State assumptions/limitations.

### Completion gate
- Main: prediction → as much 2×2 measurement as OS allows, 10 samples per viable condition, at least 2 kinds of evidence, controlled contrast, no-leak limitation.
- CS: PIE relocation vs ASLR mapping plus at least one independent `runtime - offset = image base` or inverse calculation.
- Review: Day040~050 prior Git evidence and Day153 D+2 fresh independent reasoning; new scenario/counterexample (avoid rote repeated quizzes).
- Git: input source / build flags / actual stdout CSV & notes / environment & blocked controls, a reproducible write-up, commit SHA. Teacher notes partial completion if external permission blocked; does not fabricate ASLR-OFF.
