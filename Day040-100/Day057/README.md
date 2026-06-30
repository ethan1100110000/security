# Day57 Problem Set

목표: `ret2csu`와 `xchg rax, rsp; ret` 기반 stack pivot을 복합 문제에서 복습한다.

## 문제 구성

### Problem 1 - easy
- Binary: `problem1_easy/day57_p1_easy`
- 보호기법: No PIE, No Canary, NX ON, Full RELRO
- 핵심: leak된 heap 주소를 fake stack으로 쓰고, `xchg rax, rsp; ret`로 pivot한다.
- 의도 흐름: heap fake stack -> ret2csu로 `read(0, slot, 8)` -> `[slot] = win` -> ret2csu로 `call [slot]`.

### Problem 2 - hard
- Binary: `problem2_hard/day57_p2_hard`
- 보호기법: PIE ON, Canary ON, NX ON, Full RELRO
- 핵심: FSB leak으로 canary와 PIE code pointer를 얻은 뒤, canary를 보존하고 PIE-adjusted gadget으로 `xchg` pivot + ret2csu chain을 실행한다.
- 의도 흐름: FSB leak -> PIE base 계산 -> heap fake stack 작성 -> stack BOF에서 canary 보존 -> `pop rax; ret`, `xchg rax, rsp; ret` -> ret2csu staged indirect call.

## 금지하지는 않지만 권장하지 않는 풀이

- heap에 shellcode를 넣어 실행하려고 하지 말 것. NX가 켜져 있고, 이 문제의 목표도 shellcode가 아니다.
- `leave; ret` pivot만 찾으려고 하지 말 것. 이번 문제는 `xchg rax, rsp; ret` pivot 훈련이다.
- GOT overwrite를 시도하지 말 것. Full RELRO라 GOT는 write target이 아니다. GOT는 함수 포인터를 읽거나 호출하는 용도로만 사용한다.

## 제출/정리 기준

각 문제마다 write-up에 아래 항목을 적는다.

1. 보호기법 확인
2. 입력 지점과 BOF/FSB root cause
3. heap 주소가 왜 fake stack 주소인지
4. `xchg rax, rsp; ret` 이후 `[heap]`이 왜 첫 RIP가 되는지
5. ret2csu에서 `r13/r14/r15`가 어떤 인자로 이동하는지
6. 실패 케이스 1개 이상: 예) cleanup dummy 부족, `r12=win` 착각, canary 미보존, PIE base 미보정
