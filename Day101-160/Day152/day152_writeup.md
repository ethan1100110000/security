## Day152 — W06 1/5 / x86-64 호출 규약과 스택 프레임 (진행 중)

### 2026-11-13 메인 실측 1: 함수 진입 상태
- 교사가 작성해 Git에 추가한 [`call_frame_probe.c`](call_frame_probe.c) 컴파일 설정: `gcc -O0 -g -fno-omit-frame-pointer call_frame_probe.c -o call_frame_probe`.
- 사용자의 이론 예상: 정수 인자 1~6은 RDI,RSI,RDX,RCX,R8,R9; 함수 시작 `$rsp`에는 반환 주소. 처음 `call`에서 `rsp+8`로 오해했지만 교사가 `call` 시 스택은 -8B, `ret` 시 +8B라는 방향을 설명.
- 사용자 `break *inspect` 후 GDB/Pwndbg 실제 출력:

```
rdi 0xb
rsi 0x16
rdx 0x21
rcx 0x2c
r8  0x37
r9  0x42
rsp 0x7fffffffdf78
rbp 0x7fffffffdf90
x/4gx $rsp
0x7fffffffdf78: 0x0000555555555259 0x0000000000000000
0x7fffffffdf88: 0x0000000000000000 0x0000000000000001
```

- 인자값은 각각 11/22/33/44/55/66으로 C 호출 실인자와 일치. 함수 첫 명령이 실행되기 전 상태라는 전제에서 `[rsp] = 0x555555555259`는 반환 주소로 해석. 주소가 실제 `main` 내 `call inspect` 다음 명령 위치인지 디스어셈블로 추가 대조 예정. `rbp-rsp = 0x18=24B`이지만 이는 **현재 inspect의 프레임 크기를 의미하지 않는다**. 함수 프롤로그 전에 관찰된 `rbp`는 호출자의 프레임 포인터 값일 수 있음.
- 목표: `disassemble inspect` 및 `x/i *(void **)$rsp`로 반환주소의 명령 위치 확인, 함수 프롤로그 전후의 RSP/RBP 변화 분리.
- 아직 CS 호출 규약 무힌트 반례, Day040~050 이전 스택/ROP Git 리뷰, Write-up 최종 검증은 미실시. **Day152 미완료.**


### 2026-11-13 프롤로그 어셈블리 확인 — push/mov/sub
사용자 Pwndbg 디스어셈블 제출:
```asm
0x000055555555516d <+4>: push rbp
0x000055555555516e <+5>: mov rbp,rsp
0x0000555555555171 <+8>: sub rsp,0x40
```
- 사용자는 과거 RBP 저장을 설명하고 `mov rbp,rsp`가 새 함수의 프레임 포인터가 되는 명령이라고 정확히 판단.
- **오개념 확인:** 사용자가 "rsp에 있는 복귀 주소를 rbp에 넣기"라고 말한 것은 부정확. `push rbp` 이후의 `rsp`는 이미 **저장된 옛 RBP**를 가리키므로 `mov rbp,rsp`는 해당 슬롯의 **주소**를 RBP 레지스터에 넣는다. `rbp`의 내용이 반환 주소가 아님. 반환 주소는 `[rbp+8]`에 그대로 보존됨.
- 함수 진입 초기 `rsp=0x7fffffffdf78`, `rbp=0x7fffffffdf90` 가정시: push 후 `rsp=0x7fffffffdf70`; mov 후 `rbp=0x7fffffffdf70`; sub 후 `rsp=0x7fffffffdf30`. `[rbp]`는 호출자 프레임 포인터 저장 슬롯, `[rbp+8]`은 이전에 확인한 반환 주소 `0x555555555259`.
- `sub rsp,0x40`은 로컬 프레임 공간 0x40B 확보이며, 위 주소 변화는 이 세 명령의 **예상 계산**. **아직 사용자 GDB 실행 후 실제 새 RSP/RBP 값 재제출은 없음**.


### 2026-11-13 프롤로그 중간 상태 검증 — 저장된 RBP와 복귀주소
사용자는 `push rbp; mov rbp,rsp`까지 실행한 시점으로 판단되는 Pwndbg 값을 제출했다.
```
rbp  0x7fffffffdf70
rsp  0x7fffffffdf70
x/2gx $rbp
0x7fffffffdf70: 0x00007fffffffdf90  0x0000555555555259
```
- 사용자 독립 설명: 아직 `sub rsp,0x40` 실행 전이라 RSP/RBP가 같은 값이며, 첫 슬롯은 호출자의 RBP, 다음 슬롯은 반환주소. **실측과 일치**.
- 처음 `break *inspect` 당시 RSP=0x7fffffffdf78, RBP=0x7fffffffdf90. `push rbp` 이후 `rsp=0x7fffffffdf70`, `mov rbp,rsp` 이후 RBP도 `0x7fffffffdf70`. `[rbp]`의 내용 `0x7fffffffdf90`는 호출자 저장 RBP이며 `[rbp+8]`의 `0x555555555259`는 반환주소.
- 앞서 "RBP에 복귀주소를 넣는다"는 오개념을 이번에 **사용자 스스로 정확하게 구분하여 교정**. `sub rsp,0x40`은 다음 명령으로 관찰될 것으로 예상되며 아직 이 실측 메시지에서는 해당 명령 실행 후 RSP는 확인하지 않음.

### 2026-11-13 프롤로그 마지막 변화 — sub rsp 실측 확인
- GDB `ni; info registers rbp rsp` 지시 후 사용자가 "**ㅇㅇ 맞아**"라고 답하여, 예상한 `sub rsp,0x40` 실행 뒤 `RBP=0x7fffffffdf70`, `RSP=0x7fffffffdf30`과 일치한다고 **구두 보고**했다.
- 정확한 두 레지스터의 **별도 실제 출력 텍스트는 미제출**; 이 수치는 이전 관찰과 명령 효과에서 계산한 예상값을 사용자가 일치 확인한 값으로 기록.
- 메인에서 실제로 확인한 내용: 인자 전달 레지스터 6개, 함수 진입시 [rsp] 반환주소, `push rbp / mov rbp,rsp / sub rsp,0x40`에 따른 saved RBP·return address·현재 프레임 포인터 분리 및 스택 공간 확보. `0x555555555259`가 반환주소임은 스택 구조와 실행 위치로 해석되며 `main` 디스어셈블의 후속 명령까지 직접 대조한 별도 출력은 미제출.
- **메인 필수 관찰 완료.** Day152 전체 완료에는 CS 호출규약 독립 정의/반례, 과거 Day040~050 스택/ROP Git 복습과 정리·기록이 필요함.

### 2026-11-13 CS 호출 규약: 7번째 정수 인자 1차 답변
- 교사 제시: x86-64 Linux System V ABI의 `inspect(int a,b,c,d,e,f,g)` 호출에서 실인자 10..70; 첫 6개 인자 위치, 7번째 위치, `push rbp; mov rbp,rsp; sub rsp,0x40` 뒤 RBP 상대 위치 구술.
- 사용자 첫 답: (1) `rdi rsi rdx rcx r8 r9` **정답**, (2) `r10?` **오답**, (3) `0x3c?` **오답**.
- 교사는 SysV AMD64에서 7번째 정수 인자는 보통 **호출자 스택에 전달**되며 `r10`은 7번째 정수 인자 전달 레지스터가 아니라는 점만 안내. 정확한 `[rbp+offset]`은 답을 직접 제시하지 않고 호출자가 준비한 인자 슬롯, 반환주소, 저장된 RBP의 세 위치를 사용자가 직접 추론하도록 유도.
- 이번 CS는 아직 통과 아님. 첫 답과 교정/무힌트 재검증을 구분 기록할 것.
