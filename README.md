# Security Study Roadmap

리버스 엔지니어링과 바이너리 취약점 분석을 중심으로 진행한 학습 기록이다. 성공 결과만 기록하지 않고 함수 흐름, 직접 증거, 추론 과정, 교차검증과 실패 사례를 함께 남기는 것을 목표로 한다.

현재 진도는 [`00_plan/progress_log.md`](00_plan/progress_log.md)에서 확인할 수 있다.

## Current Study Plan (2026-11-15 FINAL)

**현행 계획은 하나만 사용한다:** [Binary Exploitation 대학원 연구 준비 FINAL.xlsx](00_plan/pwn_binary_exploitation_FINAL_research_readiness_2027.xlsx).

- **핵심 방향:** Pwn 기반 Binary Exploitation / Reverse Engineering / Vulnerability Research. Fuzzing은 취약점 발견·crash triage를 위한 보조 도구.
- **실행 범위:** Day153~276 총 124개 일일 필수 과제. Day152까지의 실측·오답·Write-up은 그대로 보존. 새 계획 작성만으로 진도가 완료되지 않는다.
- **실제 진도:** [progress_log.md](00_plan/progress_log.md)의 Current Pointer가 유일한 진도 원장.
- **운영·복습·교사 원칙:** FINAL.xlsx의 `08_운영규칙`, `13_장기기억`, `14_복습실행로그`와 [review_rules.md](00_plan/review_rules.md)를 따른다.
- **필수 완료:** 메인·CS·복습·Write-up/Git 증거, 도움받은 경우 새 조건 무힌트 재검증. 지연복습은 D+2/7/21/60 기준으로 기록한다.
- **최종 준비 판정:** 독립적인 Stack/Heap/Stripped 바이너리 실기, 장기 기억 유지, 재현 가능한 연구 성과, 기술 구술. 원서·학위 마감은 학습 진도와 독립적으로 관리한다.

기존 학습계획 파일은 중복을 막기 위해 현행 브랜치에서 정리했고 Git 이력에 보존된다. [Day142~150 당시 검증 기록](00_plan/archive/day142-150_gap_audit_2026-11-12.md)은 역사적 학습 증거이지 현재 계획이 아니다.

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
