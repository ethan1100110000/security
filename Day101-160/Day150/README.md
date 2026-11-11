# Day150 — 입력 경계 조건 변경과 AFL++ 크래시 중복 분류
2026-11-11 / W05 4/5

사용자 로컬의 Day147 `archive_desk` 복사본에서 `record` 크기를 48B → 64B로 변경하고, GDB와 두 가지 73B 입력으로 Canary 경계를 검증했다. 구조체 정렬을 GCC `sizeof`/`offsetof`로 검증했으며, Day118의 AFL++ 크래시 중복 분류를 새 손 추적 조건으로 복습했다.

## 증거
- [정식 Write-up](day150_writeup.md)
- [사용자 실행 결과·해석 발췌](day150_user_observations.md)
- [record[64] 변경 참고 소스](experiment_submit64.c)
- [구조체 정렬 참고 소스](packet_layout_reference.c)
- [참고 빌드 명령](build_reproduction.sh)
- [학습 진도 로그](../../00_plan/progress_log.md)

## 범위
사용자가 직접 실행·보고한 것은 57B/73B Submit 테스트와 `packet_layout` 출력이다. 본 Git의 `experiment_submit64.c`와 `packet_layout_reference.c`는 **원격 Day147 소스와 교사 설명에서 재구성한 파일**이며 로컬 사용자 원본 파일의 직접 업로드본은 아니다. `build_reproduction.sh`도 새로 기록한 참고 스크립트로 사용자 실행 실적은 없다.

사용자의 로컬 `challenge.c`, `build.sh`, `packet_layout.c`가 이미 존재하므로 **동명의 파일은 이번 원격 커밋에 만들지 않았다.** 로컬 브랜치에서 파일 충돌이나 미커밋 변경이 없음을 확인한 후 `git pull --ff-only`한다.

GitHub의 최신 xlsx Day150 상세 과제 대조는 바이너리 파일 조회 오류로 보류했고, 별도 필수 과제가 있으면 보완한다.
