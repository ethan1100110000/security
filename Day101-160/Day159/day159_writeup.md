# Day159 — FINAL W07 3/5 — constrained ROP / ABI / control-flow proof (진행 기록)

- 실제 실습: 2026-11-25 시작, 2026-11-26 계속 (KST).
- 대상: 제공된 허가된 로컬 교육용 CTF ELF `day159_stack.elf`; stripped x86-64 PIE, NX ON, Canary OFF, Full RELRO.
- **상태: 메인 ROP 성공 / CS 기본 실습 통과 / 보안 기본 복습 통과. Day043·050 변경 조건 및 Day157·155 지연복습, 학생 원본 파일 Git 제출/검증이 남아 있어 Day 전체 미완료.**
- 근거 수준: 사용자가 채팅에 제출한 Ghidra/objdump 단편, GDB 확인 보고, 성공·실패 실행 출력. `day159_session.log`와 `exploit.py`는 사용자의 로컬에서 작성한 상태로 보고되었지만 아직 이 문서를 작성하는 시점에 학생 Git 커밋으로 검증되지 않음. 교사 측 설명을 독립 실측으로 혼동하지 않음.

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

## 7. 남은 완료 게이트

- Day043/050: 과거 Day043은 정렬 `ret` **추가**, Day050은 `ret` **제거**로 해결했음을 기록과 대조. **새 조건의 독립적인 슬롯 수 예측/재검증**은 아직 미수행.
- 지연 복습(2026-11-25 예정): Day157 D+2(PLT/GOT/lazy binding), Day155 D+7(PIE/ASLR/offset) 변경조건 질문 및 답변 아직 미수행. 해당 날짜에 못 했으므로 뒤늦게 수행해야 함.
- 학생의 로컬 `exploit.py`, `day159_session.log`, 이 write-up 내용 확인/수정과 Git 사용자 `commit/push`; push SHA 및 원본 대조 필요. 교사 작성 write-up Git 커밋은 학생 증빙을 대체하지 않음.
- 위 항목 확인 전까지 **Day159 및 FINAL W07 3/5 완료 판정 금지**.
