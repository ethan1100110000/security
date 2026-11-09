# Day148 — 독립 Pwn 진단: 메뉴 분기와 입력 경계
2026-11-09 / W05 2/5 / 사용자 직접 GDB 실습과 문답 근거

## 당일 완료 범위
- 메인: Day147 `archive_desk`에서 새 입력 조건으로 `fgets → strtol → cmp/je/jg` 분기·버퍼 크기·실제 입력 바이트 수를 분석. GDB를 통한 `2abc → RAX=2`, `abc2 → RAX=0`, 파일 입력의 개행 잔류 및 이전 버퍼 데이터 확인.
- CS: `uint8_t buf[8]`, `p=buf+3`에서 `n=6`의 `buf[8]` OOB Read, `n=5`의 `buf[7]=0x80` 조건 변경 계산.
- 복습: Day083 UAF `show`로 원래 callback `puts` leak → `edit` overwrite의 순서, `edit`를 data 영역으로 제한한 반례를 검토.
- 기록: 사용자 GDB 발췌, 해석·도움받은 부분·미확인 범위를 보존. 새로운 공격 성공이나 완전 무힌트 수행이라고 주장하지 않는다.

## 근거 문서
- [Day148 write-up](day148_writeup.md)
- [사용자 제출 GDB·문답 발췌](day148_user_observations.txt)
- [직전 Day147 대상과 빌드 방법](../Day147/README.md)
- [현재 진도](../../00_plan/progress_log.md)

## 한계
이 Day는 기존 로컬 바이너리의 **새 경계 조건 동적 검증**이다. 사용자 로컬의 전체 GDB 기록, 환경 버전/해시 및 해당 Day의 독립 C 재실행은 원격에 보존하지 않았다. CS와 힙 복습은 문답이며 사용자 힙 재실행은 없었다. 학습시간은 측정하지 않았다.

다음: **Day149 / W05 3/5** — 시작 전 최신 엑셀 Day149 과제 확인.
