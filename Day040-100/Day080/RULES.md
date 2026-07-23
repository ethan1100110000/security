# Day080 — Heap Mini Exam

## Setup

```bash
unzip day080_heap_exam.zip
cd day080_heap_exam
chmod +x setup.sh
./setup.sh
```

`setup.sh`는 현재 WSL의 gcc/glibc로 stripped ELF를 빌드한다.

## 제출 목표

1. 입력 지점과 주요 data flow
2. root cause
3. 확보 가능한 heap primitive
4. 필요한 주소 leak과 계산 검증
5. control-flow 변화 또는 shell 획득
6. 실패 케이스 1개
7. heap 상태 그림
8. 짧은 `day80_heap.md`

## 시험 규칙

- 소스는 보지 않고 `day80_exam`만 분석한다.
- assistant는 먼저 breakpoint, 입력 순서, exploit 코드, 정답 체인을 제시하지 않는다.
- 먼저 정적 분석 결과와 디버깅 계획을 제출한다.
- 우연한 성공은 완료로 처리하지 않는다.
- 주소, raw memory, allocator 상태를 서로 교차검증한다.

## 시작 제출물

```text
보호기법:
메뉴별 입력/출력 추정:
의심되는 root cause:
첫 디버깅 계획:
```
