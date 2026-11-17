# Day154 / FINAL W06 2/4 — Canary × NX 4-condition controlled experiment (in progress)

**Source of task:** FINAL XLSX `01_일일124`, Day154 (2026-11-17).
**Goal:** Compare crash location / check timing in a 2×2 Canary/NX matrix while changing only one protection setting at a time. Distinguish data corruption from code execution.

### Artifacts
- `condition_matrix.c`: teacher-prepared *local educational target*, source provided for today's **controlled** experiment. It is distinct from Day153's stripped ELF.
- `build_matrix.sh`: same source, same compiler settings except explicit stack-canary / NX toggles.
- `input20.bin`, `input80.bin`: create locally, **not yet committed**. User must record input generation and actual outputs.

### Setup (WSL/Linux)
```bash
cd /mnt/d/security-roadmap
git pull --ff-only
cd Day101-160/Day154
bash build_matrix.sh
python3 - <<'PY'
from pathlib import Path
Path("input20.bin").write_bytes(b"A" * 20)
Path("input80.bin").write_bytes(b"A" * 80)
PY
```
Running `bash build_matrix.sh` creates these four local binaries:
- `lab_canary-off_nx-off`
- `lab_canary-off_nx-on`
- `lab_canary-on_nx-off`
- `lab_canary-on_nx-on`

### Stage A: predict before executing
Read the **source** and `build_matrix.sh`. State, in your own words:
1. Which behavior should change in `overflow` when Canary toggles, holding NX constant?
2. Which behavior should change in `execute` when NX toggles, holding Canary constant?
3. Could `data` modify a value even if both defenses are on? If so, why or why not?

Do **not** claim a result before actual local tests. Answer/observations are not supplied here.

### Stage B: verify and isolate
Run the same input and mode with all 4 binaries; examples:
```bash
./lab_canary-off_nx-off overflow < input80.bin
./lab_canary-off_nx-off execute
./lab_canary-off_nx-off data < input20.bin
```
Use the parallel variants to complete the matrix. Keep compiler flags and input unchanged when comparing a single protection. Verify the actual GNU_STACK permissions from `readelf -lW`, the canary check call in disassembly, and the exact GDB stop instruction / backtrace / signal or exit code. A mere message "crashed" is insufficient.

**Note:** `data` intentionally overruns a 16B member **inside its enclosing struct**; this is a local demonstration of neighboring-field modification, not proof that a stack canary or ASan protects every subobject boundary. `execute` uses a harmless 1-byte `ret` instruction placed on the stack; no shellcode or command execution. `overflow` is intentionally unsafe and can crash; run only in the user's local test environment.

### CS and past-work review
- CS: locate and explain the canary load/check/\`__stack_chk_fail\` path and NX's role as stack page *execute* permission. Separate checking **when a function returns** from checking memory **when it is written**.
- Review: recall Day043/050 alignment + ROP observations without notes, then compare against their actual Git write-ups and derive an independent changed-condition counterexample.

### Completion criteria
- Predictions → 4 conditions → two or more distinct evidence types (code/disassembly, GDB, runtime, `readelf`).
- At least one changed-condition counterexample.
- User original input, build settings, environment, **actual** output/logs, reproducibility steps, and Write-up committed; teacher records actual progress without inferring unsubmitted data.
- Main + CS + review + Git complete. Do not complete the Day solely because the files exist.


## Stage A — user prediction (before runtime)

- 사용자 독립 사전 가설: Canary OFF에서는 오버플로우로 오염된 메모리가 이후 사용되지 않으면 바로 오류 없이 실행될 가능성이 있음. Canary ON에서는 오버플로우가 canary 슬롯을 덮었다면 함수 끝의 검사가 불일치를 감지해 `stack smashing detected`와 함께 중단될 가능성이 높음. NX ON은 스택에서 명령어 실행을 막고, NX OFF면 스택 명령 실행 가능성을 허용한다.
- 교사 검토: 큰 방향 정확. 단 **canary는 오버플로우 자체를 모두 감지하는 것이 아니라 guard가 변경되고 체크 경로를 지날 때 검출**한다. **Canary OFF가 안전한 정상 종료를 뜻하지 않으며**, 특히 이번 local 32B 대상에 80B 입력이면 saved RBP/return address까지 오염될 수 있어 `ret`에서 실패할 가능성도 큼. NX OFF는 스택 실행 권한에 대한 필요 조건 일부만 제거할 뿐 임의의 셸코드 실행 성공을 보장하지 않는다. NX ON도 스택의 데이터 변조/ROP 등을 직접 막지는 않음.
- 상태: 실측 4변형 결과 제출 전. 다음 작업은 **B (Canary OFF / NX ON)** vs **C (Canary ON / NX OFF)**에 동일 80B를 넣고 출력·signal·종료 위치 확인해 가설과 대조.
