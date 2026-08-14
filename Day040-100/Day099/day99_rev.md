# Day099 — Anti-debug 1: ptrace pattern

## 1. Entry → main

- main 후보:
- 근거:

## 2. Anti-debug 함수

- 함수 주소:
- ptrace 호출 주소:
- ptrace request와 인자:
- 반환값 검사:
- 탐지/정상 분기:

## 3. 우회 계획

- 방법 1:
- 방법 2:
- 선택한 방법과 이유:

## 4. GDB 교차검증

- PIE base:
- breakpoint:
- ptrace 직후 반환값:
- 변경한 상태:
- 우회 후 도달한 코드:

## 5. Token verifier

- 길이 조건:
- 변환/비교식:
- 목표값:
- 역산 순서:
- 최종 입력:

## 6. 실패 케이스

- 입력:
- 실패 이유:

## 7. CS — ptrace 기반 anti-debug

- 핵심 정의 2줄:
- 오늘 실습과의 연결:
- 탐지와 우회의 한계:
