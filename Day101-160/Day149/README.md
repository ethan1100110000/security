# Day149 — Submit 메모리 경계·Canary 탐지와 Day081 복습
2026-11-10 / W05 3/5

## 수행한 학습
- 메인: `archive_desk` Submit의 `record[48]`와 `rbp-0x8` Canary 사이 8바이트 padding을 분리. 48/56/57바이트 입력을 실제 실행했으며 57바이트의 Canary 첫 바이트 `00→41`을 GDB에서 비교했다.
- CS/조건 변경: 57번째 입력을 `0x00`으로 바꾸면 선언 버퍼 범위는 넘지만 Canary 비교 결과가 동일해질 수 있음을 예상했고 실제 정상 종료 확인.
- 보안 복습: Day081 glibc 2.35 `__free_hook`은 심볼/쓰기 가능성과 실제 호출 가능성을 구분. 함수 포인터 호출에 함수 인자·ABI·타입 일치가 필요하다는 반례 검토.
- 기록: 사용자가 제출한 결과·교사 도움·미확인 범위를 나눠 문서화. 세션 과제는 진행·관찰했고 무힌트 독립 전체 재현이라 표기하지 않는다.

## 근거
- [Day149 write-up](day149_writeup.md)
- [사용자 GDB·문답 발췌](day149_user_observations.txt)
- [Day147 분석 대상 소스](../Day147/challenge.c)
- [Day081 과거 Write-up](../../Day040-100/Day081/write_up.txt)
- [진도 로그](../../00_plan/progress_log.md)

## 주의
최신 `00_plan/hanyang-research-plan-2027.xlsx`의 해당 행은 이번 세션에서 GitHub 바이너리 반환의 UTF-8 디코딩 오류로 직접 대조하지 못했다. 이 기록은 교사와 사용자 사이에 실제 수행·검증한 세션 과제 범위를 나타낸다. 전체 로컬 GDB 로그·버전·해시·학습시간은 첨부되지 않았으며 별도 exploit 성공은 기록하지 않는다.
