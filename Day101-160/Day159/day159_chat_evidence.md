# Day159 — 학생 채팅 제출 실행 증거 (원본 터미널 세션 로그 아님)

**증거 종류:** 사용자가 2026-11-25~26 대화에서 직접 전사한 코드 단편, 콘솔 출력과 관찰 보고. 원래 `record.sh` 기반 `day159_session.log`가 중간에 끊겼다고 학생이 2026-11-26 보고했으므로, 이 문서는 누락 부분을 **재구성한 원시 터미널 로그로 가장하지 않음**. 학생의 로컬 `exploit.py`도 원격 Git에 제출되지 않았으며, 별도 `exploit_reconstructed.py`는 교사가 대화 기록을 기반으로 복원한 **별도 참고 구현**임.

## 1. 최초 명령 출력: 사용자 메시지 원문 인용

사용자가 `python3 exploit.py` 실행 후 제공한 출력:

```text
ethan@ddengyoon:/mnt/d/security-roadmap/Day101-160/Day159$ python3 exploit.py
[*] '/mnt/d/security-roadmap/Day101-160/Day159/day159_stack.elf'
    Arch:       amd64-64-little
    RELRO:      Full RELRO
    Stack:      No canary found
    NX:         NX enabled
    PIE:        PIE enabled
[+] Starting local process './day159_stack.elf': pid 213
b'telemetry=0x5c9d268c3199\n'
0x5c9d268c3199
[*] Loaded 7 cached gadgets for './day159_stack.elf'
PIE Base : 0x5c9d268c2000
pop_rdi  : 0x5c9d268c31a3
pop_rsi  : 0x5c9d268c31a8
ret      : 0x5c9d268c3016
target   : 0x5c9d268c32f7
Payload length: 136
[+] Receiving all data: Done (107B)
[*] Process './day159_stack.elf' stopped with exit code 2 (pid 213)
push one raw packet (max 208 bytes); LF/CR are rejected
packet accepted
CTF_FAIL: argument register values
```

**실패 원인에 대한 학생의 사후 자기 진단 (원문):** `pop rdi를 두번씀 rsi가 아니라 오타냄`.

학생의 수정 설명에 의하면, `pop rsi;ret` 대신 `pop rdi;ret`를 두 번째 가젯으로 잘못 넣었다. 대상은 먼저 진입 스택 정렬을 검사하고 이어서 RDI/RSI 값을 검사하므로, 이 출력은 ABI 인자 실패를 가리킨다.

## 2. 수정 후 성공: 사용자 메시지 원문 인용

사용자는 'payload 실수'를 고친 후 아래 출력을 제출함.

```text
[+] Receiving all data: Done (121B)
[*] Process './day159_stack.elf' stopped with exit code 0 (pid 225)
push one raw packet (max 208 bytes); LF/CR are rejected
packet accepted
CTF_OK: two-argument control-flow proof accepted
```

성공 출력에는 실행 파일에 존재하는 control-flow gate의 인자·정렬 검사를 통과한 경우에만 출력되는 `CTF_OK`가 포함되어 있다. 이는 **학생이 실행한 출력의 채팅 전사**이지, 누락된 터미널 출력의 복원본이나 교사 측 재실행 결과가 아님.

## 3. 그 외 사용자 직접 확인 사항 (원본 콘솔 전체 아님)

- 정적 분석: `read(0, DAT_00104040, 0xd0)` 이후 LF/CR 바이트 검사, `0x133c`에서 `memcpy` 길이 검증 누락, 정상 경로 `packet accepted`. 초기에는 이 출력의 성공 여부를 혼동했으나, 이후 함수를 분리하여 통제 흐름 표식을 찾음.
- `objdump`의 `write@plt` 호출 주소 (사용자 원문): `0x1224`, `0x1291`, `0x12e8`, `0x13a9`, `0x1476`, `0x152f`.
- Ghidra에서 `0x11b1` 성공 출력 함수, `0x1233` 실패 출력 함수, `0x12f7` 검사 분기 식별. 검사: `RSP&0xf == 8`, `RDI=0x1337133713371337`, `RSI=0x4242424241414141`.
- 사용자 보고 GDB: 96바이트 `A`를 넣고 `memcpy` 전후 `[rbp+8]`이 `0x4141414141414141`로 덮인 것 확인. 스택 지역 버퍼 시작 `[rbp-0x50]`에서 saved RIP까지 오프셋 `0x58`.
- 사용자 가젯 검색 원문: `0x00000000000011a3 : pop rdi ; ret`, `0x00000000000011a8 : pop rsi ; ret`. 런타임 단독 `ret`는 `pie base+0x1016`이라고 첫 실행 출력에서 확인됨.
- 학습 중 pwntools `p.recvline()`, `ROP(elf).find_gadget([...]).address`, `p64`와 `flat` 문법은 교사의 도움으로 교정함. 최초 인자 가젯 오타는 학생이 직접 식별·수정.
- CS: RBX가 callee-saved이고 RDI는 caller-saved임을 이해, GDB에서 진입/반환 시 RBX 보존·RDI 변경 확인을 사용자 구두 보고. call/ret의 RSP ±8 원리 설명.
- 복습: NX와 코드 재사용, Full RELRO와 스택 저장 반환주소, PIE와 같은 프로세스 누출 설명. Day043/050 변경 가젯 정렬 질문, Day157 D+2 PLT/GOT와 BIND_NOW, Day155 D+7 두 주소의 PIE base 계산 통과. 자세한 답변의 도움 여부는 `day159_writeup.md` 참조.

## 4. 증거 한계와 학습 판정 원칙

1. 이 기록은 대화에서 **실제로 제출된 출력만** 인용한다. 없는 `day159_session.log`의 손실 구간은 새로 만들어 채우지 않는다.
2. 학생의 최종 로컬 `exploit.py` 전체 바이트는 Git에서 직접 확인하지 못했다. 따라서 `exploit_reconstructed.py`는 정확한 원본 SHA나 로컬 실행 증빙을 제공하지 않는다.
3. 성공 프로그램의 게이트 구조와 사용자 출력으로 목표 경로 실행을 판단할 수 있다. 다만 성공한 **원본 페이로드 바이트 전체**와 매 실행의 금지 바이트 단언 실행 여부는 검증하지 못했다.
4. 후속 로컬 원본 파일이 남아 있으면 별도 커밋으로 추가할 수 있지만, **원본이 사라졌다는 이유만으로 실습을 처음부터 반복하라고 요구하지 않는다.**
