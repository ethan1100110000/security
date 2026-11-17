# Day154 — FINAL W06 2/4 — Canary × NX 4조건 통제 실험 (2026-11-17 완료)

> 환경: 사용자 WSL/Linux, x86-64, GCC, GDB/Pwndbg, objdump, readelf. 실험물은 교사가 제공한 로컬 교육용 [`condition_matrix.c`](condition_matrix.c)와 [`build_matrix.sh`](build_matrix.sh). 사용자 실측 원문과 구두 일치 확인을 구분한다.

## 목적과 통제변수
- 하나의 소스에서 **Canary ON/OFF** × **NX ON/OFF**의 4가지 실행 파일을 생성, 동일한 입력을 투입해 변인 하나씩 비교한다.
- 각 실험의 핵심은 다음으로 분리한다.
  1. `overflow`: 32바이트 로컬 버퍼에 `A×80`을 쓰려는 경우 Canary의 실패 경로와 NX 영향.
  2. `execute`: 별도 오버플로우 없이 스택의 1바이트 `0xC3`(`ret`)를 함수 포인터로 실행하려는 경우 NX 영향.
  3. `data`: `struct { unsigned char bytes[16]; uint64_t marker; }`에서 20바이트 입력으로 인접 `marker` 필드를 덮는 경우의 Canary/NX 한계.
- 빌드 설정은 공통 `-O0 -g -fno-omit-frame-pointer -fno-pie -no-pie`, Canary OFF `-fno-stack-protector` / ON `-fstack-protector-all`, NX OFF `-Wl,-z,execstack` / ON `-Wl,-z,noexecstack`. `build_matrix.sh`가 네 변형을 자동 빌드.

## 재현 절차와 입력
```bash
cd /mnt/d/security-roadmap
git pull --ff-only
cd Day101-160/Day154
bash build_matrix.sh

python3 - <<'PY'
from pathlib import Path
Path("input80.bin").write_bytes(b"A"*80)
Path("input20.bin").write_bytes(b"A"*20)
PY
```
- `input80.bin` = `0x41` 80회, `input20.bin` = `0x41` 20회. 길이/값은 사용자 실습에 사용한 생성 명령과 일치한다.
- 명령: `./lab_canary-{off,on}_nx-{off,on} overflow < input80.bin`의 4경우, `./lab_canary-off_nx-{off,on} execute`, `./lab_canary-on_nx-on data < input20.bin`.
- 체크: `objdump -d -M intel lab_canary-on_nx-on`, `readelf -lW lab_canary-off_nx-off`, `readelf -lW lab_canary-off_nx-on`, `gdb ./lab_canary-off_nx-on`.

## 사전 예측 — 사용자 독립 추론
- Canary OFF에서 오버플로우가 일어나도 언제나 즉시 오류가 나는 것은 아니며, 사용하지 않는 메모리의 오염은 정상 종료처럼 보일 수 있다. 다만 이번 80B 복사는 저장된 복귀 주소 손상까지 가능하므로 충돌 가능성은 높다.
- Canary ON은 guard 슬롯을 덮고 함수 반환 직전 검사에 도달하면 stack smashing 검사로 중단될 가능성이 높다. **모든 경계 초과를 자동 탐지하는 것은 아님.**
- NX는 스택 페이지에서 명령어 실행을 막는 기능이며, 입력이 단순 `A` 반복인 경우 `ret`의 비정규 복귀 주소 오류에 직접적인 차이를 만들지 않을 것으로 예상했다.
- `execute`에서는 NX OFF 시 1바이트 스택 명령어 실행·복귀, NX ON 시 실행 권한 오류를 예상. NX ON이라고 `0xC3` 스택에 **저장**하는 일이 금지되지는 않는다고 독립 설명.

## 80바이트 오버플로우 — 보호 설정별 결과
| Canary | NX | 사용자 결과 | 제출 수준 |
|---|---|---|---|
| OFF | OFF | `Segmentation fault` | 실행 후 '예상대로' 구두 확인 |
| OFF | ON | `Segmentation fault (core dumped)` | 터미널 출력 원문 및 GDB ret 오류 위치 구두 확인 |
| ON | OFF | `*** stack smashing detected ***: terminated` → `Aborted (core dumped)` | 터미널 출력 원문 |
| ON | ON | `stack smashing detected` → Aborted | 실행 후 '예상대로' 구두 확인 |

사용자가 직접 제출한 B/C 원문:
```text
[Canary OFF, NX ON]
[overflow] enter up to 80 bytes
[overflow] read=80 first=0x41
Segmentation fault (core dumped)

[Canary ON, NX OFF]
[overflow] enter up to 80 bytes
[overflow] read=80 first=0x41
*** stack smashing detected ***: terminated
Aborted (core dumped)
```
- **해석:** Canary OFF/NX ON에서 GDB로 `RIP`가 `ret` 명령을 가리키고, `[RSP]`가 `0x4141414141414141`인 상태여서 잘못된 반환 대상 사용 중 실패했다고 사용자가 보고. `RSP` **자체**와 `[RSP]`의 8B 데이터는 구분해야 한다. 새 GDB 주소 원문은 미제출이므로 중단 위치는 학생의 설명/구두 실측으로 기록.
- Canary ON에서는 오염된 스택의 카나리 불일치 검사로 `__stack_chk_fail` 경로를 거쳐 abort되므로 손상된 복귀 주소가 실제로 사용되기 전 탐지한다.
- 이 입력에서는 NX ON/OFF 변경만으로 종료 분류가 달라지지 않았다. **NX는 일반적으로 무의미한 것이 아니라 이번 오류가 명령어 가져오기보다 반환 주소 처리 단계에서 난다는 뜻**.

## NX 실제 동작 — 별도 오버플로우 없는 스택 1바이트 `ret` 실행
- 코드: `volatile unsigned char stack_code[16] = {0xc3};` 다음 배열 시작 주소를 함수 포인터로 변환하여 호출한다.
- 사용자가 Canary OFF로 고정하고 `NX OFF` vs `NX ON`의 `execute` 모드를 실행한 뒤 사전 예상과 일치한다고 **구두 확인**.
- 의미: 이 실험에서 NX OFF는 스택 명령어 `0xC3` 실행 후 복귀 가능, NX ON은 스택의 실행 권한이 없어 실행을 차단. **사용자 터미널 출력/중단 신호 전문은 공유되지 않았으므로 구두 확인으로 분류**.
- NX ON에서도 스택 쓰기·일반 데이터 사용은 허용된다는 사용자 독립 설명 확인.

## 구조체 멤버 인접 데이터 오염 — 보호 설정 둘 다 ON
```c
struct frame_data {
    unsigned char bytes[16];
    uint64_t marker;
};
```
`marker` 초기값: `0x1122334455667788`, 입력 `0x41`×20B.

학생이 직접 확인한 최종 출력의 핵심: **`marker=0x1122334441414141`**.
- 리틀엔디언 순서에서 `bytes[16]`을 넘어 쓰는 4B가 `marker`의 하위 4B를 `0x41414141`로 바꿈. 상위 4B는 기존 `0x11223344` 유지.
- 동일 구조체의 다른 필드를 변경했을 뿐 별도의 스택 Canary를 손상시키지 않아 `stack smashing detected`가 나타나지 않음. NX는 데이터 수정에 관여하지 않음.
- 이 코드는 `bytes[16]`이라는 멤버의 경계를 넘겨 쓰는 **의도적으로 안전하지 않은 실습 코드**이고, **구조체 전체의 24B를 넘어선다는 뜻은 아님**.

## CS: 디스어셈블과 ELF `GNU_STACK` 근거
학생이 `overflow_probe`에서 직접 찾아낸 핵심:
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
- 사용자가 독립적으로 `fs:0x28`에서 원본 TLS Canary를 읽고 `[rbp-0x8]`에 저장, 반환 직전 비교해 같으면 `leave;ret`, 다르면 `__stack_chk_fail` 호출이라고 정확히 설명했다. `sub`가 0이면 ZF=1이어서 `je`가 실행된다.
- 학생의 `readelf -lW` 보고: **NX OFF → `GNU_STACK RWE`**, **NX ON → `GNU_STACK RW`**. 원문 전체 표는 미제출이나 실행 실험과 일치하는 실행 권한 비트 차이를 정확히 설명함.

## Day043/Day050 누적복습: ROP 정렬 조건 변경
- [Day043 원문](../../Day040-100/Day043/write_up.txt): `ret` 가젯을 추가해서 `win` 진입 RSP 정렬을 교정.
- [Day050 원문](../../Day040-100/Day050/write_up.txt): `ret` 가젯을 제거해서 `win` 진입 RSP 정렬을 교정. 정렬 문제는 `ret` 가젯을 무조건 추가하는 방식으로 해결하는 것이 아님.
- **새 문제:** `pop rdi; ret` 가젯 진입 직후 `RSP%16=8`, 인자값과 `win` 주소를 스택 순서대로 배치. `win` 진입 목표 `RSP%16=8`.
- 학생의 독립 답: "올바르지않나? pop ret이 각각 rsp + 8을 하니까" — **정답**. `pop rdi`는 RSP를 8B 올려 `8→0`, 이어서 `ret`도 8B 올려 `0→8`. 따라서 별도 `ret` 가젯 추가가 필요하지 않다. 추가한다면 목표 정렬이 깨질 수 있다.
- 이번 조건은 전날 단순 `sub rsp` 오프셋 퀴즈가 아니라 **두 종류 가젯 명령이 소비하는 스택 슬롯 수**를 계산한 새로운 조건이다.

## 결론·증거 한계·진도
- 메인: Canary 2x2 비교 및 GDB 실패 원인 확인, NX 스택 실행 제한 실험, Canary/NX가 모두 켜져도 구조체 필드 데이터 오염 가능함을 실측. 사용자 사전 예상 → 실측 비교의 흐름 완료.
- CS: 실제 Canary 저장/검사 명령어와 `GNU_STACK` 권한 차이를 학생이 찾아 설명하여 완료.
- 복습: Day043/050 Git 원문 대조, 새로운 `pop rdi; ret` 조건에서 힌트 없이 정렬이 맞는다는 결론 및 원인 확인 완료.
- 기록: 소스·빌드·입력 생성법·주요 출력/주관적 보고의 구별·디스어셈블·한계·오답 없이 Git 기록 완료.
- 한계: 학생 출력 원문은 B/C 오버플로우 경우와 `marker` 수치 중심이며 나머지는 구두 예측 일치 기록. 보호기법의 존재가 모든 메모리 오염을 막는다는 결론은 도출하지 않는다. **Day154 당일 4영역 완료**; 이후 새로운 바이너리에서 독립 실험 및 지연 복습으로 전이 검증.
