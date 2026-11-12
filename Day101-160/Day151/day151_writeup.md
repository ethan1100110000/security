# Day151 — W05 5/5 / 독립 분석 진도 기록
2026-11-12 / 새 로컬 실습: [snapshot_queue.c](snapshot_queue.c) / **Day 진행 중**

## 메인 — 실제 사용자 분석 및 ASan 근거 (초안)
- 새 소스는 교사가 제공함. 사용자는 소스를 보고 1(New) / 2(Close) / 3(Recent) / 4(Exit) 제어 흐름과 힙 할당·해제 및 두 전역 포인터 `active`, `last_preview`를 추적했다.
- 사용자가 처음에는 `create_snapshot`의 `free(active); active=new_snapshot;` 부분을 잘못 의심했으나, 기존 active를 해제한 다음 새 할당 주소를 할당하므로 이후 `active->id` 접근 자체는 문제 없음을 교정했다.
- 사용자는 이후 메뉴 `1 → 2 → 3`에서 `active`와 `last_preview`가 같은 객체를 가리켰다가 `close_snapshot`에서 `active`만 NULL로 초기화되고 `last_preview`는 해제된 주소를 유지하여 UAF로 이어질 수 있음을 스스로 도출했다.
- ASan 컴파일이 안내되었고 사용자가 다음 **실제 실행 출력**을 제출. 사용자의 실제 입력 title/detail 문자열은 제시되지 않았으므로 재현 절차는 메뉴 흐름 1(New, title/detail 입력)→2(Close)→3(Recent)로 기록함.

```text
==131==ERROR: AddressSanitizer: heap-use-after-free on address 0x50c000000040
READ of size 4 at 0x50c000000040
#0 show_latest .../Day151/snapshot_queue.c:60
#1 main .../Day151/snapshot_queue.c:75

0x50c000000040 is located 0 bytes inside of 116-byte region
[0x50c000000040,0x50c0000000b4)

freed by thread T0 here:
#1 close_snapshot .../Day151/snapshot_queue.c:50
#2 main .../Day151/snapshot_queue.c:74

previously allocated by thread T0 here:
#1 create_snapshot .../Day151/snapshot_queue.c:24
#2 main .../Day151/snapshot_queue.c:73

SUMMARY: AddressSanitizer: heap-use-after-free .../snapshot_queue.c:60 in show_latest
==131==ABORTING
```

- `Snapshot` 객체: `id` 4B + `title` 32B + `detail` 80B = **116B**. 보고된 `READ of size 4`는 객체 시작(오프셋0)의 `id`를 읽는 `last_preview->id`와 일치.
- ASan의 `fd` shadow bytes는 해제된 영역임을 나타냄. 할당→해제→접근 시점을 세 개의 스택트레이스로 구분할 수 있음.
- **검증 범위:** 실제 힙 UAF 읽기 오류 ASan 탐지·프로그램 중단 입증. 임의쓰기·코드 실행·악용 성공은 이 기록으로 주장하지 않음.

## 남은 Day151 필수 과제
- 메인: 사용자가 자기 말로 root cause 및 수정 방향을 요약한 후 수정 코드의 **정상 동작/같은 재현 입력 시 UAF 미탐지**를 비교할 것. 수정 코드를 아직 적용/실행한 증거 없음.
- CS: 계획표 정수 변환·배열 관련 **3문항 구술**, 틀린 항목 무힌트 재검증, 오답 근거 기록.
- 보안 2회독: **과거 Pwn 풀이** 회상→Git 원문 대조→변경 조건 반례 재설명.
- Git: 실제로 수행한 결과, 사용자 설명과 교사 도움, 제한 사항을 확정하여 Day151 커밋/로그 갱신. 현재 완료 판정 **보류**.
