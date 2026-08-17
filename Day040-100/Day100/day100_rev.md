# Day100 — Anti-debug 2: timing / trap

## 1. Entry → main

- main 후보:
- 근거:

## 2. Timing check

- 시간 함수:
- 시작/종료 측정 위치:
- 측정 구간:
- 시간 차 계산식:
- threshold:
- 탐지 분기:

## 3. Trap check

- handler 주소:
- handler가 변경하는 상태:
- handler 등록 함수:
- INT3 주소:
- INT3 이후 검사:
- 탐지 분기:

## 4. 우회 계획

- timing 우회:
- trap 우회:
- 선택한 방법과 이유:

## 5. GDB/objdump 교차검증

- PIE base:
- timing breakpoint와 관찰값:
- SIGTRAP 발생 시 GDB 상태:
- flag 또는 signal 처리:
- 우회 후 도달한 코드:

## 6. Token verifier

- 길이 조건:
- 비교식:
- 목표값:
- 역산 순서:
- 최종 입력:

## 7. 실패 케이스

- 입력:
- 실패 이유:

## 8. CS — timing/trap 기반 anti-debug

- 핵심 정의 2줄:
- 오늘 실습과의 연결:
- 오탐과 우회 가능성:
