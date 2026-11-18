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


## Stage A — student independent predictions

- 학생이 ASLR ON 전제에서 10회 재시작 시 예상한 결과: **PIE OFF**이면 `main` 고정, `libc` 랜덤, `stack` 랜덤; **PIE ON**이면 `main` 랜덤, `libc` 랜덤, `stack` 랜덤. 같은 실행 파일 내 `main - helper` 오프셋은 실행 간 **고정**. **이론상 정확한 사전 가설**.
- 보완 조건: 이는 정상적인 Linux ASLR 활성화, PIE/Non-PIE 빌드 설정, 매번 **별도 프로세스**로 다시 실행하는 환경이란 전제. ASLR ON에서도 libc/stack의 실제 다양성은 10회 측정해 확인해야 하며, 상대 오프셋 고정은 동일 바이너리의 동일 로딩 이미지 기준.
- 현재 상태: 학생 독립 예상만 제출됨. `measure_aslr-on.csv`/원본 출력·회수 수치는 아직 없음. 다음 `python3 measure.py --aslr on --runs 10`을 사용자 로컬에서 수행 후 `unique-values` 요약을 제출받아 가설 검증할 예정.


## Stage B — user WSL ASLR-ON 10-run measurement summary

- 2026-11-18 사용자 WSL에서 `python3 measure.py --aslr on --runs 10` 실행 후 **직접 제출한 요약 원문**:
```text
no_pie trials=10 unique-values: {'main': 1, 'helper': 1, 'stack': 10, 'libc_puts': 10, 'main_minus_helper': 1}
pie trials=10 unique-values: {'main': 10, 'helper': 10, 'stack': 10, 'libc_puts': 10, 'main_minus_helper': 1}
```
- 사전 예측과 **완전히 일치**: PIE OFF 실행 파일의 main/helper 고정; PIE ON의 main/helper는 10회 중 각각 10개 주소; stack/libc_puts는 두 설정 모두 10개 주소; 동일 빌드의 main/helper 상대 차이는 1종.
- 사용자 제출은 요약 통계이며 **CSV의 실제 주소 샘플/개별 offset 값은 아직 제출되지 않음**. 10회 주소가 서로 달랐다는 사실만으로 ASLR 총 엔트로피/무누출 익스플로잇 성공 확률을 계산할 수 없음.
- 다음: 사용자가 PIE ON에서 측정한 `main` 절대 주소 1개와 `nm -n addr_pie`의 `main` 심볼 값 1개를 직접 가져와 `PIE_base = runtime_main - ELF_main_offset`을 독립 계산. 가능하면 다른 프로세스도 대조. 이후 ASLR OFF 권한 테스트는 실제 허용 여부에 따라 결과/거부 원문 기록.


## Stage C — independently calculated PIE load base

- 사용자 **실제 로컬 출력 원문**:
```text
main=0x5f4c76b3519c helper=0x5f4c76b35189 stack=0x7ffd90f0fc40 libc_puts=0x7e4177880e10 main_minus_helper=0x13
$ nm -n addr_pie | grep ' main$'
000000000000119c T main
```
- 사용자 독립 계산:
  - runtime `main=0x5f4c76b3519c`, ELF `main` symbol value `0x119c`.
  - `PIE_load_bias = 0x5f4c76b3519c - 0x119c = 0x5f4c76b34000`, **정확**.
  - `helper` ELF offset = `0x5f4c76b35189 - 0x5f4c76b34000 = 0x1189`, **정확**.
  - `main-helper = 0x119c - 0x1189 = 0x13`, 실측 `main_minus_helper=0x13`과 일치.
- Interpretation: `nm`의 ET_DYN 심볼 값은 PIE 로딩 기준에 대한 값으로 이 샘플에서는 `runtime = load_bias + ELF_symbol_value`. 한 실행의 이 값은 다음 ASLR 재시작에서 달라질 수 있다. `main-helper` 거리는 바뀌지 않는다.
- 학생이 중간 힌트 없이 base/offset을 직접 계산하여 CS 오프셋 산술 요건 충족.
- 다음: `python3 measure.py --aslr off --runs 10`을 로컬에서 시도해 허용 여부 확인하고, 실패하면 오류 전문을 기록하며 정상 결과를 만들어내지 않는다. 이어 PIE ON/OFF의 address uncertainty 측정된 유니크 개수/표와 no-leak 시나리오 비교, Day153 D+2과 Day040~050 복습, write-up.


## Stage B — user WSL ASLR-OFF 10-run measurement summary

- 사용자 독립 예측: "aslr이 꺼져있으면 pie가 어떻든 싹다 고정이지" — **주소 다양성 기준으로 정확한 예측**.
- 학생이 WSL에서 `python3 measure.py --aslr off --runs 10` 실행하여 제출한 **실제 도구 출력 요약**:
```text
saved: measure_aslr-off.csv
no_pie trials=10 unique-values: {'main': 1, 'helper': 1, 'stack': 1, 'libc_puts': 1, 'main_minus_helper': 1}
pie trials=10 unique-values: {'main': 1, 'helper': 1, 'stack': 1, 'libc_puts': 1, 'main_minus_helper': 1}
```
- **2×2 통제 비교, 조건마다 10회 사용자 측 반복 실행 완료**:
  | PIE | ASLR | distinct main | distinct helper | distinct stack | distinct libc_puts | distinct main-helper |
  |---|---|---:|---:|---:|---:|---:|
  | OFF | ON | 1 | 1 | 10 | 10 | 1 |
  | ON | ON | 10 | 10 | 10 | 10 | 1 |
  | OFF | OFF | 1 | 1 | 1 | 1 | 1 |
  | ON | OFF | 1 | 1 | 1 | 1 | 1 |
- `measure_aslr-off.csv`는 **학생 로컬에서 실제 생성된 파일**, 원본 CSV는 아직 Git 리포지토리에 업로드되었음을 확인하지 않음. `measure_aslr-on.csv` 역시 로컬 생성되었으나 원문 전체는 아직 미제출. 40 프로세스 실행의 고유 주소 개수 결과로 한정.
- 결론: PIE는 실행 파일이 재배치될 수 있도록 하고, ASLR 활성화가 해당 재배치를 주소 무작위화한다. ASLR OFF에서는 측정한 10회가 모두 같은 주소; 다른 환경/빌드에 일반화하지 않는다.
- 다음: 표를 통한 10-run 표본의 address diversity 수치와 **leak 없는 조건의 불확실성** 논증. 10개 서로 다른 PIE 베이스가 보였다고 성공률이 `1/10`이라는 결론을 내릴 수 없음(모집단/엔트로피 미확정). 학생 독립 답변 후 Day153 D+2/Day043~050 복습, write-up 작성.


## Stage C — no-leak address uncertainty (student answer)

- 교사 조건: PIE ON, ASLR ON, `main` ELF 오프셋 `0x119c`, `helper` 오프셋 `0x1189`, 런타임 주소 출력/누출 없음. 질문 (1) helper 오프셋만으로 절대주소 산출 가능 여부, (2) 10회 PIE Base가 각기 달랐으므로 임의 추측 성공률=1/10 추정의 타당성.
- 학생 독립 응답: **(1)** "오프셋을 알아도 base를 모르니까 base를 더한 실제 주소 유출이 필요함" — 정확. 엄밀하게는 **동일 PIE 로딩 이미지의 런타임 포인터 하나**가 유출되어 그 포인터의 알려진 오프셋을 뺄 수 있다면 base를 구할 수 있음(반드시 helper 포인터 누출일 필요는 없음). **(2)** "아니지" — 정확. 이유 설명은 아직 없으므로 심층 근거는 교사 설명을 통해 확인; 학생 독립적으로 엔트로피를 계산했다는 증거는 아님.
- 10회씩 각 구성에서 **서로 다른 주소 1개 or 10개**가 나타났다는 것은 표본 다양성일 뿐, 가능한 전체 PIE Base 수·분포·독립적인 재시작 공격 성공률을 알려주지 않음. 관측된 10개 중 하나를 선택한다고 `P=1/10`을 결론 내릴 수 없음.
- PIE ON/ASLR ON의 blind no-leak 가정과 이번 측정 프로그램의 실제 주소 출력(의도적인 measurement leak)을 구별해야 함.
- Next: Day153 D+2 challenge with parser copy offsets `b=+0x3c`, `a=+0x3e` under a new length to test byte-selective local-field overwrite without saved return address corruption; then Day040~050 Git-linked altered-condition review and final write-up.
