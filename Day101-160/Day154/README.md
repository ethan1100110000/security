# Day154 / FINAL W06 2/4 — Canary × NX 4-condition controlled experiment (완료: 2026-11-17)

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


## Stage B 80-byte overflow matrix: user confirmation

- 2026-11-17 사용자 보고: 이전에 예측한 나머지 **Canary OFF/NX OFF → SIGSEGV**, **Canary ON/NX ON → `stack smashing detected` / Aborted**가 로컬 실행에서 **"예상대로임"**이라고 일치 확인됨. 두 새 조건의 *실제 터미널 출력 전문*은 미제출이므로 **구두 결과 일치 확인**으로 기록.
- 4조건 80B overflow 표: OFF/OFF → SIGSEGV(구두 일치), OFF/ON → `ret`에서 SIGSEGV(터미널 및 GDB 원인 설명), ON/OFF → stack smashing detected/Aborted(터미널 전문 제출), ON/ON → stack smashing detected/Aborted(구두 일치).
- 결론: 이 입력에서 NX만 변경해서는 오류 종류가 바뀌지 않았고, Canary 설정이 탐지 경로의 변화를 만들었다. **NX가 모든 상황에서 무관하다는 일반화는 금지**. 이제 `exec_probe()`를 이용해 스택 명령어 실행 시도에서 NX ON/OFF 차이를 분리해 관찰한다. 이후 `data_probe()`로 인접 데이터 변경과 Canary의 한계를 확인할 것.

## Stage C prediction: stack execution probe
- 학생 독립 설명: `exec_probe`가 스택 배열에 `0xc3`(x86-64 `ret` 명령)를 저장하고 이를 실행하려 하며, **BOF 없이** NX ON/OFF의 명령어 실행 허용 차이를 확인하는 실험이라고 정확히 이해함. NX ON도 스택에 `0xc3`을 저장하는 **쓰기 작업은 가능**하며, 실행 권한 없는 스택 페이지에서 명령어 실행을 시도할 때 문제가 된다고 설명.
- 사전 가설: NX OFF에서 함수 포인터를 통한 스택 `ret` 실행 성공 예상, NX ON에서는 실행 권한 문제로 오류 예상. **실행 결과는 아직 제출되지 않음**.
- 다음은 Canary OFF를 고정하고 `./lab_canary-off_nx-off execute` / `./lab_canary-off_nx-on execute` 관찰, 필요하면 GDB에서 잘못된 메모리 접근 주소/권한을 대조.

## Stage C user-local NX exec probe (verbal confirmation)
- 사용자 WSL에서 `./lab_canary-off_nx-off execute`와 `./lab_canary-off_nx-on execute`를 각각 실행하고 **"실행해봤어 예상대로네"**라고 결과가 사전 가설과 일치했다고 구두 보고.
- 사전 가설은 NX OFF일 때 스택 내 1바이트 `ret` 실행 후 반환 가능, NX ON일 때 스택 실행 시도에서 오류 발생. **정확한 출력/시그널/GDB 중단 위치의 원문은 제공되지 않았으므로 구두 일치 확인에 한정**. `exec_probe`는 BOF 없는 별도 명령 실행권한 실험임.
- 다음 과제: Canary ON/NX ON으로 고정해 `data_probe`의 `struct {bytes[16]; marker;}`에 20바이트를 넣고, marker 변화 및 가드 비손상 여부를 확인하여 **Canary의 탐지 범위가 스택의 모든 데이터 변경에 미치지 않음**을 실측.


## Stage D: struct neighbor modification with both protections ON

- 학생 로컬 실험 결과: `./lab_canary-on_nx-on data < input20.bin` 실행에서 `marker=0x1122334441414141` 확인(이번 사용자 메시지 직접 보고).
- 학생 독립 설명: `bytes[16]` 다음의 `marker`로 20바이트 쓰기의 초과 4바이트가 넘어가고, 보호용 Canary 슬롯을 변경하지 않으므로 stack smashing 검사가 해당 데이터 변조를 알아채지 못한다고 판단. **핵심 원리 및 리틀엔디언 결과 정확**. `marker`의 하위 4바이트는 `41 41 41 41`, 상위 4바이트는 기존 `0x11223344`가 유지됨.
- 정밀 조건: `bytes` 멤버의 16B 경계는 넘었지만 별도 스택 카나리를 건드리지 않고 **같은 struct 안의 다음 필드를 수정**한 예. Canary는 모든 경계를 추적하는 도구가 아니며 NX는 해당 데이터 수정과 무관. `read`가 `bytes` 멤버 크기보다 길게 쓰는 동작은 안전한 C 코드가 아니고, 두 보호기법이 켜져도 막히지 않을 수 있음.
- 상태: **80B overflow의 4조건 비교, NX 스택 명령 실행 구두 확인, 보호기법 ON에서도 인접 필드 오염 실측**까지 메인 실험 진행. CS 보호코드/메모리 퍼미션 실제 근거 및 별도 전이/복습, Day154 write-up/Git은 진행 예정. 완료 처리 전.


## CS user evidence: canary initialization and check in overflow_probe

- 사용자 스스로 `lab_canary-on_nx-on`의 `objdump -d -M intel`에서 발견한 `overflow_probe` 프롤로그/에필로그 Canary 경로:
```asm
4012dd: mov rax,QWORD PTR fs:0x28
4012e6: mov QWORD PTR [rbp-0x8],rax
...
40134b: mov rax,QWORD PTR [rbp-0x8]
40134f: sub rax,QWORD PTR fs:0x28
401358: je 40135f
40135a: call 4010d0 <__stack_chk_fail@plt>
40135f: leave
401360: ret
```
- 사용자 독립 해석: 프롤로그에서 `fs:0x28`의 원본 TLS Canary를 `[rbp-0x8]`에 저장, 반환 전 저장 Canary와 원본 Canary를 비교, 같으면 `leave;ret`, 다르면 `__stack_chk_fail@plt` 호출. **분석 정확**.
- 정밀화: `sub` 결과 0이면 ZF=1이 되어 `je` 분기로 검사 통과, 0이 아니면 실패 호출. 검사 시점은 `leave;ret` 직전이며 복사/쓰기 순간을 가로막는 검사가 아님. CS Canary 어셈블리 근거 확인 완료.
- 다음 CS 조건 확인: `readelf -lW`에서 `GNU_STACK` 플래그를 NX ON/OFF 변형끼리 비교하여 실행 권한 `RWE` vs `RW` 관찰. NX 입력·동작 구두 증거와 정적 권한 근거를 연결. 이후 복습 및 Day154 Write-up.

## CS evidence: NX GNU_STACK ELF header (user observation)
- 사용자 직접 `readelf -lW` 비교 보고: `nx-off` 바이너리의 `GNU_STACK` 플래그는 **`RWE`**, `nx-on`은 **`RW`**. 따라서 ON 설정에서는 GNU_STACK에 실행 권한 **E**가 없음을 정확히 설명.
- 사용자 원문은 플래그/의미에 대한 요약이며 readelf 전체 출력은 미제출. 앞서 `exec_probe` NX ON/OFF 동작 비교와 정적 헤더 증거가 연결되어 **Day154 CS 보호기법 구현 확인 완료**.
- 다음 Day043/050 누적복습에서는 **`pop rdi; ret` 가젯과 단독 `ret`가 각자 소비하는 스택 슬롯 수가 다른 새 조건**에서 목표 함수 진입의 `RSP%16`을 판단한다. 이전날 했던 동일 `sub rsp` 문답은 반복하지 않는다. 사용자 독립 답변 및 설명 후 기록.

## Day043/050 changed-condition review: pop rdi; ret
- 교사 제시 무힌트 새 반례: `pop rdi; ret` 가젯 진입 시 `RSP%16=8`, 인자값과 `win` 주소가 순서대로 스택에 있으며 목표 `win` 진입 `RSP%16=8`. 학생 답 "올바르지않나? pop ret이 각각 rsp + 8을 하니까" — **정답 및 핵심 근거 독립 제시**.
- 계산: 진입 8 → `pop rdi` (+8B) 후 0 → `ret` (+8B) 후 8. 따라서 `win` 진입의 ABI 기준을 충족하므로 **단독 `ret` 추가 불필요**. 추가 `ret`은 이 조건에서는 8→0으로 바꾸어 잘못 정렬한다.
- 과거 Git [Day043](../../Day040-100/Day043/write_up.txt)은 `ret` 추가로 성공, [Day050](../../Day040-100/Day050/write_up.txt)은 `ret` 제거 후 성공. 보호 여부가 아니라 목표 함수 진입 시 RSP 정렬에 따라 결정해야 함.
- 이 문제는 전날 단순 `sub rsp` 계산 반복이 아니라 **POP+RET 가젯의 두 슬롯 소비**를 적용한 새 조건의 독립 답변. Review 조건 충족.

## Final Day154 status
- 80B overflow 보호 조합 4개(두 개 출력 원문/두 개 구두 일치), GDB `ret` 오류 위치 구두 확인, stack command execution NX 실험 구두 확인, 20B 구조체 멤버 초과 쓰기/marker 실측, Canary 검사 코드 objdump 학생 원문, NX `GNU_STACK` RWE/RW readelf 학생 요약, ROP 정렬 새로운 조건 정답까지 수행.
- [Day154 Write-up](day154_writeup.md)에 실제/구두 관찰의 경계를 보존해 기록. **메인·CS·복습·Git 4영역 완료 (2026-11-17)**. 전 범위 터미널 원문/신호 값 미제출, data member UB와 구조체 경계 사이 구분 등 한계를 기록.
