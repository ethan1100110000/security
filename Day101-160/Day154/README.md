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


## Stage B — first controlled user-local run B vs C

- 사용자가 동일 `input80.bin` (`A`×80)를 `overflow` 모드로 각 빌드 실행한 **WSL 실측 원문**:
```text
[B: Canary OFF / NX ON]
[overflow] enter up to 80 bytes
[overflow] read=80 first=0x41
Segmentation fault (core dumped)

[C: Canary ON / NX OFF]
[overflow] enter up to 80 bytes
[overflow] read=80 first=0x41
*** stack smashing detected ***: terminated
Aborted (core dumped)
```
- 비교: 두 변형 모두 `read`에서 80B를 읽고 첫 바이트 0x41을 확인함. **Canary OFF**에서는 SIGSEGV를 의미하는 셸 메시지가 출력되지만, 정확한 예외 지점/원인(손상된 복귀 주소 등)은 **아직 GDB 실측 전이라 가설**. **Canary ON**에서는 stack protector 검사 실패에 따른 중단 메시지와 `Aborted`(일반적으로 SIGABRT)가 관찰됨. NX 조건도 동시에 다르므로 이 두 관찰만으로 NX 인과관계를 완전히 분리하지 말고 동일 NX에서 Canary만 변경한 다른 쌍도 비교할 것.
- 사용자 사전 예상의 'Canary OFF에서는 메모리 오염 후에도 오류가 나지 않을 수 있음'은 **일반 원리로 맞지만 이 80B 실험에서 정상 종료로 이어지지는 않음**. Canary ON 검사 탐지 예상은 확인됨.
- 다음 검증: Canary OFF/NX ON에 GDB로 중단 지점 `RIP`, `x/i $rip`, `RSP`, backtrace를 확인한 뒤, 상대 조건을 통제하여 off/off 및 on/on도 실행. 실제 출력을 받기 전 4조건 완료로 표시하지 않음.

## GDB confirmation: Canary OFF / NX ON return-site failure
- 사용자 GDB 후속 구두 보고: Canary OFF / NX ON `overflow < input80.bin`에서 `RIP`는 `ret` 명령을 가리키고, 복귀 대상으로 사용할 스택의 슬롯에 `0x4141414141414141`이 기록되어 있어 예외가 발생함을 직접 확인했다고 설명.
- 정밀 구분: **`RSP` 레지스터 자체가 0x4141…이라는 뜻이 아니라 `[RSP]`에 저장된 8바이트 값이 0x4141…**이어야 `ret`가 이 값을 복귀 주소로 사용하려 한다는 설명이 성립. 사용자 공유는 주소/레지스터 원문 덤프가 아닌 구두 실측 해설이며, 그 범위만 검증 기록.
- 이에 따라 B 조건의 SIGSEGV 위치가 `ret`임을 사용자 보고로 확인. 다음은 같은 80B 입력에 `lab_canary-off_nx-off`와 `lab_canary-on_nx-on` 두 조건을 실행해 Canary/NX 통제 비교를 완성할 것.

## Stage B prediction before final two variants
- 사용자 독립 예측(미실험): 동일한 `A×80` 오버플로우에서 NX ON/OFF 변화만으로는 중단 방식에 실질적인 차이가 없을 것으로 예상. Canary ON/OFF가 보호 경로(`__stack_chk_fail` vs 손상 복귀주소 `ret`)의 차이를 결정할 것으로 설명.
- 판단: 현재 입력은 실행 가능한 코드가 아니라 `0x41` 반복이며 `0x4141414141414141`은 비정규 반환 대상이므로, 해당 위치로 명령어를 가져오는 NX 검사 전에 반환 과정에서 실패할 것으로 예측하는 근거가 타당함. **NX가 항상 무관한 게 아니며 스택 코드 실행이 시도될 경우 별도로 영향**.
- OFF/OFF, ON/ON **실제 실행 출력은 아직 미제출**. 이후 결과와 예측을 대조 예정.
