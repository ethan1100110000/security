# Day161 — FINAL W07 5/5 — 블라인드 ELF 독립 평가 (진행 중)

- 시작: 2026-11-27 (KST). **학생 실측 성공/ROP 성과: 아직 없음.**
- 평가 기준: [README](README.md)와 FINAL 계획의 W07 5/5. 교육용 stripped ELF. 교사가 준비한 바이너리는 학생의 독립 탐지 성과가 아니다.

## 1. 학생 최초 실행 출력 (채팅으로 제출, 원본 Git 세션 확인 전)

학생 원문:

```text
session=0x02df2f4577c8c93a
trace[0]=0x5be72258428d
trace[1]=0x790e07280e10
trace[2]=0x4227
one opaque packet (max 208 bytes; CR/LF rejected)
frame accepted
```

학생 첫 관찰: `주소같은게 나오넹` (주소처럼 보인다는 가설. 각 필드가 주소인지, 어떤 매핑의 주소인지, 재현 시 어떻게 변하는지는 아직 검증 안 됨.)

교사 관찰 분류(학생 증거 아님): `trace[0]`·`trace[1]`는 64비트 포인터 형태를 띠지만 실측 매핑/반복 실행 없이 ELF/libc로 확정하지 않는다. `session`은 16진수 식별값일 수 있고 `trace[2]`는 작은 수로 정수·offset·부분 주소 등 미확정. `frame accepted`는 단순 입력 수락이며 목표 기능 실행 성공으로 인정하지 않는다.

## 2. 다음 확인

- 학생이 처음 분석한 `file`, `checksec`, `readelf` 결과 및 2~3회 독립 실행 출력 제출.
- 학생 스스로 각 필드의 출처 가설과 어떻게 검증할지 설명.
- 추후 실제 Git `day161_session.log` 원본 확인 후 정적·동적 근거, CS·복습·재현 20회·실패 매트릭스를 구분해 기재.
