# Day140 — push/pop 실제 관찰과 호출 스택

- 학습일: 2026-10-23 (Asia/Seoul)
- 계획: W03 바이너리 기초 / 4/5. Day141과 같은 날 진행.
- 메인·CS·복습 실제 시간은 측정하지 않았다.

## 질문과 예상

Day139의 저장값/레지스터 혼동을 실제 작은 실험과 연결했다. rbx=100을 push한 뒤 rbx=200으로 바꾸면 현재 레지스터는 200, 스택 저장값은 100이라고 예상했다. pop 후 rbx=100, rsp는 8 증가한다고 답했다.

## 소스와 실행

GDB와 LLDB는 현재 환경에 없어 사용하지 않았다. 별도 어셈블리 함수 stack_probe가 레지스터와 스택 메모리의 값을 출력 배열에 기록하고 C 드라이버가 검사했다. GDB 단일 단계나 기존 copy_data의 실행 관찰이 아니다.

```bash
cc -std=c11 -Wall -Wextra -Werror -O0 -g stack_probe_test.c stack_probe.S -o stack_probe_test
./stack_probe_test
```

컴파일·실행 exit 0. 원자료는 stack_probe_results.txt.

| 지점 | rsp | rbx | 실험 저장값 |
|---|---|---:|---:|
| 실험 push 전 | 0x00007FFE9B75C320 | 100 | — |
| push 후 rbx 변경 | 0x00007FFE9B75C318 | 200 | [rsp]=100 |
| 실험 pop 후 | 0x00007FFE9B75C320 | 100 | — |

세 검사(push의 rsp-8, pop의 rsp 복구, 값 복구)가 PASS였다. 주소는 이번 실행의 값이며 ASLR 등에 따라 달라질 수 있다.

stack_probe의 첫 push와 마지막 pop은 실제 호출자의 rbx를 보존하기 위한 바깥 저장/복원이다. 표는 그 안의 실험 push/pop을 관찰한다. C 드라이버가 실제 호출자 rbx 보존까지 별도로 검증한 것은 아니다. 어셈블리는 GNU AT&T 문법이다.

## CS — 스택 정렬과 call/ret

일반 System V AMD64 호출에서 call 직전 rsp는 16바이트 정렬 상태이고, call이 8바이트 복귀 주소를 스택에 넣으면 함수 진입 시 rsp는 8 감소한다. ret은 그 주소로 복귀하면서 rsp를 8 증가시킨다.

사용자는 0x...C318에서 sub rsp,8 후 0x...C310을 맞혔다. 이어지는 call에서 0x...C318을 선택해 감소 방향을 교정했다. call 직전 0x2000이면 진입 시 0x1ff8이라는 예제는 설명을 받았다. call이 값을 넣는 동작임과 함수의 추가 스택 변경이 없다면 call/ret 뒤 원래 rsp로 돌아감을 확인했다. 정렬과 call/ret은 문답·손 추적이며 새 동적 측정은 없다.

## 복습·한계·다음

push로 저장한 복사본과 이후 레지스터 변경은 독립이다. 저장값을 사용한 뒤 복원해야 한다. push rbx에는 대응하는 복원이 필요하다고 답했다.

이번 실행은 직선형 push/pop 경로만 다뤘다. 여러 분기의 공통 복원 블록, call/ret 내부, 기존 copy_data의 실제 분기는 관찰하지 않았다. sanitizer·퍼징·성능 측정·5분 무힌트 발표는 없다. call 방향과 복원 시점은 다음 복습에 남긴다.

다음: Day141 / W03 5/5 — 반복문 소스·최적화된 주소 비교 대응, 실행 결과와 증명 한계.
