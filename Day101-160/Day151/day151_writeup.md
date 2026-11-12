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

## 메인 수정·동일 입력 ASan 재검증 (2026-11-12)
- 사용자 최초 수정 제안: `close_snapshot()`에서 `free(active)`뿐 아니라 `free(last_preview)`도 수행한 후 NULL 대입. 교사는 **두 포인터가 같은 heap 객체를 가리키므로 double-free가 발생**할 수 있다고 지적.
- 사용자 스스로 수정: `last_preview = NULL;`만 추가(대문자 `NULL`은 교사 교정). 기존 `free(active); active=NULL;`를 유지하므로 할당 객체는 **한 번만 해제**, 해제된 주소에 대한 `last_preview`의 dangling reference를 제거함.
- **사용자 로컬에서 수정 후 ASan 재빌드·실행 결과** (사용자 실제 출력):
```text
1) New  2) Close  3) Recent  4) Exit
> 1
title> asdf
detail> asdfasdf
created #1

1) New  2) Close  3) Recent  4) Exit
> 2
closed

1) New  2) Close  3) Recent  4) Exit
> 3
no recent snapshot
```
- 사용자: "이번엔 uaf가 안뜨고 정상적으로 실행댐"이라고 명확히 보고. **동일 메뉴 경로에서 ASan UAF가 사라지고 분기 메시지가 바뀐 것은 검증**, 다른 메뉴 경로나 전체 프로그램 무결성을 보장하는 것은 아님.
- [원본 취약 소스](snapshot_queue.c)는 재현을 위해 보존. [수정 참고 소스](snapshot_queue_fixed_reference.c)는 사용자 변경 내용을 **교사가 원본에서 재구성한 별도 파일**이며 사용자 로컬 파일의 byte-identical 업로드를 주장하지 않음. 로컬 수정 원본은 여전히 사용자의 환경에 있음.

## Root Cause 정리 및 결과
- **원인:** `create_snapshot()`이 `active`와 `last_preview`를 같은 동적 할당 객체로 향하게 함. `close_snapshot()`은 `free(active)` 후 `active = NULL`만 실행하여 `last_preview`를 dangling pointer로 남김. `show_latest()`는 NULL만 확인하고 `last_preview->id`를 읽어 **해제된 힙 객체를 접근**.
- **재현 경로:** `1(New) → 2(Close) → 3(Recent)`. ASan `heap-use-after-free READ4`, `show_latest:60`에서 보고; `create_snapshot:24` 할당, `close_snapshot:50` 해제라는 추적 정보로 입증.
- **수정:** `close_snapshot`이 객체를 해제한 뒤 `last_preview = NULL`을 추가함(이 프로그램의 단일-활성-Snapshot 상태 모델에서 유효). 같은 객체에 대한 `free`를 두 번 호출하지 않음.
- **비교 검증:** 수정 전 `==131==ABORTING`; 수정 후 메뉴 3에서 `no recent snapshot` 정상 출력, 해당 시나리오의 ASan UAF 미검출.
- **학습 독립성:** 사용자는 정상 메뉴·alias·Close→Recent 문제를 스스로 확인했으며, ASan 실행도 사용자 수행. `create_snapshot` 자유 포인터 문제로 처음 오해한 점과 double-free를 유발할 수 있는 최초 수정안은 교사의 지적 후 보완. 이력 감추지 않음.
- **한계:** 이 메인 과제의 재현/수정은 완료했으나 Day151의 CS 3문항·과거 Pwn 복습과 전날/오늘 Git 증빙 확인은 아직 남아 Day 전체는 완료 아님.

## 2026-11-12 CS 3문항 — 설명·오답 교정

**조건:** 64-bit Linux (포인터 8B, int 4B)를 가정, 정수 변환과 배열의 독립 구술 3문항. 사용자의 최초 구술 답과 질문 후 교정을 구분한다.

1. `void check(char buf[32]) { sizeof(buf); }`: 사용자 최초 "32바이트, char=1B"라고 답함 — **오답**. 함수 매개변수 `char buf[32]`는 `char *buf`로 조정되므로 `sizeof(buf)=8`. 교사가 이 규칙을 설명함.
2. `int index=-1; size_t count=8; index<count`: 사용자는 `size_t` 기준 unsigned 변환으로 -1이 큰 unsigned가 되므로 **NO**라고 판단 — 정답.
3. `char buf[16]; size_t len=16; if(len<=sizeof(buf)) buf[len]='\\0';`: 사용자는 최대 유효 인덱스가 15인데 16에 쓴다고 설명 — 정답, 1바이트 OOB Write.

**배열/포인터 새 조건 재시험:** `void inspect(int data[100]) { sizeof(data); }`의 A, `int numbers[100]; sizeof(numbers)`의 B 질문.
- 최초 독립 반례 답 `A=4,B=100` — **오답**. 각각 요소의 크기와 개수를 혼동.
- 교사가 함수 매개변수 포인터/실제 배열, 64-bit 포인터8B, int4B라는 조건을 재안내.
- 사용자 최종 정정 `A=8,B=400` — 정확. 이는 **힌트 후 교정 답안**이며 완전 무힌트 최종 통과라고 주장하지 않는다.
- 정밀 근거: A는 `int *` 매개변수이므로 `sizeof=8`; B는 실제 100원소 배열이고 `100 * 4 = 400B`이다. 두 값이 다른 이유는 배열 매개변수 조정 규칙 때문.

**평가:** 3문항 중 최초 2개 정답, 1개 오류. 새 조건 첫 시도 오답, 교정 설명 후 정답 도출. 학습 내용과 교정 증빙은 확보하되 **엄격 계획의 "오답 무힌트 재시험" 독립 합격 증거는 아직 부족**하므로 추후 짧은 새 조건 하나만 재확인한다. 반복적으로 같은 예제를 다시 묻지 않는다.

## 남은 Day151 필수 과제
- 메인: **완료** (원인·ASan 재현·수정·동일 경로 비교; 사용자 최초 오답·교사 도움 구분 기록).
- CS: **3문항 구술 실시**. 최초 2/3 정답, 오답 1문항의 새 반례는 최초 오답 후 힌트받아 8/400으로 정정. 엄격한 무힌트 재시험 완료 증거는 아직 없음(짧은 새 조건 한 문제 보류).
- 보안 2회독: 이전 Pwn 풀이 회상→Git 원문 대조→변경 조건 반례 재설명.
- Git: 각 보완 항목과 실제 사용자 산출물을 기록. 원본/수정 참고본과 Write-up은 저장소에 커밋 중이며 사용자 로컬 수정 원본 commit 여부는 확인되지 않음. **Day151 전체 미완료**.
