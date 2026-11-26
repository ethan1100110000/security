# Day160 — FINAL W07 4/5 — 보호기법 통제 실험 (진행 중)

- 학습 시작: 2026-11-26 (KST)
- 상태: 초기 가설 제출, 실측·CS·복습·학생 로그 미완료.
- 실험 설계: 동일 `lab160.c`에서 기본 Full RELRO/PIE ON/NX ON과 각각 한 설정만 바꾼 Partial RELRO, PIE OFF, NX OFF 변형을 비교. 세부 과제는 [README](README.md)를 따른다.

## 1. 실행 전 학생의 초기 가설 — 채팅 원문

질문: ① Full→Partial RELRO에서 GOT 쓰기 권한, ② PIE OFF + ASLR ON에서 `main` 주소 변화, ③ NX OFF에서 스택 코드 실행과 ROP 필요성.

학생 답변 원문: `1. write 가능 2. 안바뀜 3. 실행은 가능 rop필요할 수 있음`.

- 잠정 평가: 세 방향 모두 타당하나 ①에서 모든 GOT 슬롯이 writable인 것은 아니며 일반적으로 lazy binding을 사용하는 `.got.plt`와 RELRO 보호된 `.got`를 구별해야 함. ②는 일반적인 Linux non-PIE main 코드에 대한 예측으로, libc/stack ASLR 비활성화를 뜻하지 않음. ③은 스택 실행 권한이라는 한 조건만 바뀐 것이며, 임의 분기 제어나 제약 조건을 해결했다고 뜻하지 않음.
- **아직 빌드·readelf·GOT 쓰기·주소 반복·스택 명령 실행의 학생 측 실측 출력 미제출.** 예상 결과를 측정 결과처럼 기록하지 않는다.

## 2. 다음 작업

사용자는 `bash build.sh; bash record.sh` 이후 정적 ELF 특성으로 각 파일의 설정을 확인하고, 먼저 기본형의 `file`/`checksec`와 `readelf` 결과를 수집한다. 이후 4개 조건의 3회 주소 관측, 동일값 GOT 슬롯 쓰기, 스택에서 1바이트 RET 실행을 비교하고 사전 가설과 대조한다. 이전 Day043/050 정렬, Day158 D+2 주소 누출 신뢰도, CS Full/Partial RELRO 복습 후 원본 로그·커밋을 확인한다.
