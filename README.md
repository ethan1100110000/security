# Security Study Roadmap

리버스 엔지니어링과 바이너리 취약점 분석을 중심으로 진행한 학습 기록이다. 성공 결과만 기록하지 않고 함수 흐름, 직접 증거, 추론 과정, 교차검증과 실패 사례를 함께 남기는 것을 목표로 한다.

현재 진도는 [`00_plan/progress_log.md`](00_plan/progress_log.md)에서 확인할 수 있다.

## Reversing Portfolio

| Day    | 주제                                        | 문서                                          |
| ------ | ----------------------------------------- | ------------------------------------------- |
| Day090 | stripped 바이너리의 함수 경계 복원                   | [write-up](Day040-100/Day090/write_up.txt)  |
| Day098 | indirect call과 함수 포인터 테이블 분석              | [write-up](Day040-100/Day098/write_up.txt)  |
| Day099 | `ptrace` 기반 anti-debug 분석과 동적 우회          | [write-up](Day040-100/Day099/write_up.txt)  |
| Day100 | timing·trap 기반 anti-debug 분석              | [write-up](Day040-100/Day100/write_up.txt)  |
| Day101 | 조건 분기의 파일 offset 계산과 정적 패치                | [write-up](Day101-160/Day101/write_up.txt)  |
| Day105 | Ghidra 기반 Stack BOF와 exploitability 판단    | [write-up](Day101-160/Day105/write_up.txt)  |
| Day107 | strings/imports와 caller 도달 가능성을 이용한 정적 분석 | [write-up](Day101-160/Day107/write_up.txt)  |
| Day109 | stripped 바이너리의 함수 흐름·BOF·decoy 종합 분석      | [write-up](Day101-160/Day109/day109_rev.md) |

## Analysis Workflow

```text
entry에서 main 복원
→ imports·strings·XREF 조사
→ caller/callee와 data flow 복원
→ 위험한 입력·메모리 연산 확인
→ 보호 기법과 exploitability 구분
→ GDB·objdump로 핵심 추론 교차검증
→ 실패 사례와 분석 한계 기록
```

## Portfolio Checkpoint

8개 리버싱 문서의 품질 점검 결과와 이후 문서 작성 기준은 [Day110 checkpoint](Day101-160/Day110/day110_rev.md)에 정리했다.
