# Day159 — FINAL W07 3/5 — constrained ROP / ABI / control-flow proof (진행 기록)

- 실제 실습: 2026-11-25 시작, 2026-11-26 계속 (KST).
- 대상: 제공된 허가된 로컬 교육용 CTF ELF `day159_stack.elf`; stripped x86-64 PIE, NX ON, Canary OFF, Full RELRO.
- **판정: Day159 / FINAL W07 3/5 학습 완료 (원본 터미널 세션 유실에 대한 명시적 증빙 대체 승인).** 학생의 채팅 제출 원문 실패/성공 출력과 설명으로 핵심 수행을 판정했으며, 원본 `day159_session.log` 및 로컬 `exploit.py` 전체 바이트는 검증하지 못했음.
- 근거 수준: 사용자가 채팅에 제출한 Ghidra/objdump 단편, GDB 확인 보고, 성공·실패 실행 출력. 학생은 기록용 세션이 중간에 끊겼다고 보고. [학생 채팅 제출 증거](day159_chat_evidence.md)에 실제 전사 출력을 인용했으며, [재구성 스크립트](exploit_reconstructed.py)는 **교사 작성 참고 구현**이다. 원본 `day159_session.log`나 원본 `exploit.py`인 척하지 않는다.

## 1. 정적 재구성 및 예측

사용자 `checksec` 관찰: `amd64-64-little`, Full RELRO, Canary 없음, NX ON, PIE ON. 프로그램은 `telemetry=0x...` 주소를 출력한 뒤 최대 `0xd0`(208)바이트의 단일 입력을 요구. `read`는 ELF 오프셋 `0x4040`의 전역 버퍼를 사용하며, 입력 중 `0x0a` 또는 `0x0d` 발견 시 `packet rejected: prohibited byte` 후 반환. 이 검사는 `memcpy` **이전**에 수행됨.

`0x133c` 함수: `memcpy` 목적지 `[rbp-0x50]`, 원본 전역 입력 버퍼, 복사 길이로 `read` 반환값을 사용. 입력 최대 길이 208바이트는 지역 버퍼 및 반환 주소까지의 거리보다 큼. 해당 함수가 출력하는 `packet accepted`는 정상 입력 수락 메시지이며 ROP 성공 증거는 **아님**.

사용자가 Ghidra로 `write@plt` 호출 위치 `0x1224/0x1291/0x12e8/0x13a9/0x1476/0x152f`를 분류하여 숨은 경로를 탐색함.
- `0x11b1`: `CTF_OK: two-argument control-flow proof accepted` 출력 후 `_exit(0)`.
- `0x1233`: 오류 원인별 `CTF_FAIL: entry stack alignment` 또는 `CTF_FAIL: argument register values` 출력 후 `_exit(2)`.
- `0x12f7`: 목표 진입 게이트. `RSP & 0xf == 8`, `RDI == 0x1337133713371337`, `RSI == 0x4242424241414141` 조건을 검사하고 통과 시 `0x11b1`로 점프.

## 2. 동적 RIP 덮어쓰기 근거

사용자는 `/tmp/day159_A96.bin`에 `A` 96바이트를 저장. GDB `starti`, `break *$rebase(0x1373)`(memcpy 전), `break *$rebase(0x1378)`(후)에서 `[rbp+8]`을 비교하고 반환 주소가 `0x4141414141414141`로 바뀌는 것을 확인했다고 보고. 지역 목적지 `[rbp-0x50]`에서 saved RIP `[rbp+8]`까지 거리는 `0x58 = 88`바이트. 초기 가설을 동적 관찰로 교차 검증함.

## 3. PIE와 체인

`telemetry` 출력은 ELF `0x1199` 코드 위치에 대응한다고 사용자가 Ghidra로 확인. 같은 프로세스에서 `pie_base = leak - 0x1199`를 계산함. 성공 이전 실행 예시:

```text
telemetry=0x5c9d268c3199
PIE Base : 0x5c9d268c2000
pop_rdi  : 0x5c9d268c31a3
pop_rsi  : 0x5c9d268c31a8
ret      : 0x5c9d268c3016
target   : 0x5c9d268c32f7
Payload length: 136
```

가젯의 ELF 오프셋은 `pop rdi; ret = 0x11a3`, `pop rsi; ret = 0x11a8`, 단독 `ret = 0x1016`. 런타임 주소는 `elf.address = pie_base` 설정 후 `ROP(elf)`에서 가져옴. Python의 `p.recvline()`으로 누출 문자열을 받고 `int(...,16)`로 파싱. `flat()` 또는 `p64()`로 패킹하는 법을 교사의 도움으로 재확인함.

최종 페이로드 순서:

```text
88 bytes padding
pop rdi; ret
0x1337133713371337
pop rsi; ret
0x4242424241414141
ret
PIE base + 0x12f7
```

전체 `88 + 6*8 = 136`바이트. 전송 전 `0x0a/0x0d` 금지 바이트와 `len(payload) <= 208`을 검사하도록 안내받았고 `send()`를 사용. 실제로 사용한 전체 `exploit.py` 코드 및 실행당 금지 바이트 검사 원문은 학생 Git 원본 확인 대기.

## 4. 통제된 실패 → 수정 → 성공

사용자 최초 출력:

```text
packet accepted
CTF_FAIL: argument register values
Process exited with code 2
```

사용자가 직접 원인 확인: `pop rsi; ret`를 입력할 자리에 `pop rdi; ret`를 두 번 사용한 오타. 첫 `pop rdi`로 목표 RDI를 설정했으나 두 번째 `pop rdi`가 RSI용 상수로 RDI를 덮어쓰고 RSI를 준비하지 못함. 스택 정렬 실패가 아니라 인자 검사 실패였으므로 이 실행에서는 정렬 분기가 통과한 것으로 해석. 이후 오타 수정, 사용자 두 번째 실행 출력:

```text
packet accepted
CTF_OK: two-argument control-flow proof accepted
Process exited with code 0
```

두 실행은 사용자 직접 수행/제출. 독립 GDB 레지스터 최종 스냅샷은 제출되지 않았으므로 목표 함수의 ABI 조건 충족은 **게이트 검사 및 출력 결과로 입증**한 것으로 기록.

## 5. CS — SysV AMD64 ABI

- `RBX/RBP/R12–R15`: callee-saved. `RDI/RSI` 등 인자 레지스터는 caller-saved.
- 학생은 `push rbx; mov rbx,0x1234; mov rdi,0x5678; pop rbx; ret`의 입력 `RBX=0xAAAA, RDI=0xBBBB`에서 출력 `RBX=0xAAAA, RDI=0x5678`을 올바르게 도출.
- 학생이 안전한 짧은 입력으로 `0x133c` 함수의 GDB 진입/복귀를 비교하여 RBX 보존, RDI 변경, RSP 변경을 보고(레지스터 원본 숫자는 채팅에 미제출).
- `call`은 반환주소를 push하여 RSP를 8 감소, `ret`은 8 증가. 정상 호출 예시 `call` 직전 `0x7fffffffe000` → 함수 진입 `0x7fffffffdff8` → 복귀 `0x7fffffffe000`을 학생이 독립 설명.
- 정상 함수 진입 `RSP % 16 = 8`, `call` 직전 `RSP % 16 = 0`. `pop rdi;ret` 및 `pop rsi;ret`은 각 16바이트를 소비해 modulo-16 정렬을 유지하고, 단독 `ret`은 8만큼 전환.

## 6. 보안 복습 및 독립성

- NX: 스택은 실행 불가지만 ROP 스택에는 주소/데이터가 있고 실행된 명령어는 기존 ELF 실행 코드 영역임을 학생 독립 설명. 통과.
- Full RELRO: GOT 포함 관련 영역의 쓰기 제한과 스택 반환주소 overwrite를 구분. 학생 독립 설명. 통과.
- PIE/ASLR: 바이너리 내부 오프셋은 실행마다 동일, ASLR ON에서 로드베이스가 바뀔 수 있어 **프로세스 A 누출을 B에 재사용하면 통상 실패**함을 독립 설명. 단 '반드시' 실패는 과장: PIE 자체는 난수화 보장이 아니며 ASLR이 켜져도 같은 베이스가 우연히 재선택될 수 있음(교사 교정).

## 7. Day043·050 변경 조건 및 지연복습 (2026-11-26)

### Day043/050 — ROP 정렬 변경 조건

학생에게 첫 가젯 진입 `RSP%16=8`, 단일 `pop rdi; pop rsi; ret` 체인을 새로 제시. 학생 최초 답은 "추가 ret 필요할 듯"으로 **결론 정답**. 다만 `+24B`, `8 → 0`, 단독 `ret`으로 `0 → 8`이라는 **수치 계산은 교사가 설명함**; 무힌트 독립 수치 계산으로 과장하지 않음. 이전 Git 기록의 Day043 ret 추가 / Day050 ret 제거와 비교했으며, 상황에 따라 가젯 소비 슬롯 수가 달라진다는 요점을 학습.

### Day157 D+2 — PLT/GOT 지연 바인딩

- 학생은 처음 호출 전 GOT 슬롯이 `puts@plt+6`처럼 PLT 내부를 가리킨다는 질문에 "lazy binding"이라고 스스로 답함.
- 변경 조건: `main` 전, 해당 PLT 함수 호출도 없는데 GOT가 libc 주소이면 `BIND_NOW`가 먼저 주소를 기록하기 때문이라고 독립 답함.
- 즉시 해결과 지연 해결의 시점을 구분함. 통과.

### Day155 D+7 — PIE base와 누출의 출처

새 주소 두 개: `A=0x555555557250` (ELF offset `0x1250`), `B=0x5555555575a0` (ELF offset `0x15a0`). 학생은 두 PIE base 모두 `0x555555556000`으로 계산해 일치함을 독립 답함. 서로 다른 base가 나온다면 다른 프로세스의 leak이거나 동일 PIE 이미지 주소가 아닐 수 있다고 독립 설명. **조건은 '같은 프로세스'로 명시되어 있었기에 다른 실행의 leak 가설은 이 문제에서 제외**; 누출 주소 출처/오프셋 대응 오류 또는 서로 다른 매핑을 우선 검토하고 vmmap과 비교해야 한다는 교사 교정.

### 판정 및 미완료 사항

- 기본 보안 복습, ABI CS, 변형 복습, 만기 복습 모두 대화에서 실시함. 정렬 변형의 수치 도출은 교사 도움을 받았다고 명시하여 독립 숙달로 과장하지 않음.
- 학생의 로컬 `exploit.py`와 `day159_session.log` 원본은 Git 미제출 상태이며 녹화 세션 유실을 학생이 보고. 원본 로그의 모든 명령/페이로드, 금지바이트 검사 실제 실행 여부는 확인 불가. 재구성 코드는 제출된 대화 구조를 토대로 한 교사 작성 참고물이고 **학생이 이 파일을 실행했다고 주장하지 않는다**.
- **최종 판정: Day159 / W07 3/5 학습 완료 (2026-11-26).** 학생이 직접 제출한 실패/성공 출력 및 원인 진단, 정적/동적 분석 보고, CS 및 지연복습으로 수행을 입증했고, 원시 세션 기록이 끊긴 사실을 별도 명시해 증빙 공백을 공개했다. 교사가 [채팅 증거](day159_chat_evidence.md)와 [구분된 재구성 스크립트](exploit_reconstructed.py)를 Git에 커밋했다. **원본 프로그램 실행 전체 로그·원본 스크립트의 무결성은 검증 불가**이며 독립 원본 검증으로 간주하지 않는다. 교육 진행은 Day160으로 이동; 원본 증거가 나중에 발견되면 보강.
