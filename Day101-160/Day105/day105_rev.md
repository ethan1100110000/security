# Day105 - Stripped Mini Exam

## 1. 보호 기법

- Architecture:
- PIE:
- NX:
- RELRO:
- Stack Canary:
- Stripped:
- 근거:

## 2. main 복원

- ELF entry:
- main 후보:
- 판단 근거:

## 3. 함수 흐름

```text
entry
└─
```

| 임시 이름 | 주소/offset | 역할 | caller | 근거 |
|---|---:|---|---|---|
|  |  |  |  |  |

## 4. 입력 함수 비교

| 호출 위치 | API | 목적지 | 관찰된 공간 | 요청 길이 | 도달 조건 | 판단 |
|---:|---|---|---:|---:|---|---|
|  |  |  |  |  |  |  |

## 5. 취약점 및 offset

- 취약 함수 후보:
- buffer 시작:
- Canary 위치:
- saved RBP 위치:
- return address 위치:
- buffer → Canary:
- buffer → RIP:
- 최대 입력 뒤 ROP 여유 공간:
- 계산 근거:

## 6. 안전한 호출 / decoy 구분

- 후보:
- 실제 호출자 존재 여부:
- 안전 또는 decoy라고 판단한 이유:

## 7. Exploitability 계획

- 현재 제공되는 primitive:
- 보호기법별 장애물:
- 추가로 필요한 leak/primitive:
- 조건부 1단계 payload:
- 조건부 2단계 payload:
- 현재 가능한 영향:

## 8. llvm-objdump / llvm-readelf 교차 검증

```bash
# 사용한 명령
```

- 확인한 사실:

## 9. 결론

- 취약점 유형:
- 핵심 근거 한 줄:
- 분석 중 가장 헷갈린 점:
