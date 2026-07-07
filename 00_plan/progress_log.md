# Progress Log

이 파일은 새 채팅에서도 바로 이어가기 위한 기준 파일이다.
매일 공부가 끝나면 아래 세 가지와 운영 규칙을 기준으로 갱신한다.

- Last completed: 마지막으로 완료한 Day
- Current focus: 지금 이어갈 주제
- Next task: 다음에 바로 시작할 작업

---

## Operating Rule

공부 시작 전:
- GitHub repo를 확인한다.
- 사용자는 먼저 아래 명령을 실행한다.

```bash
cd /mnt/d/security-roadmap
git pull
```

공부 끝난 후:
- 사용자는 해당 Day 폴더에 산출물을 정리한다.
- 사용자는 아래 흐름으로 산출물을 commit/push한다.

```bash
cd /mnt/d/security-roadmap
git status
git add .
git commit -m "complete DayXXX"
git push
```

그 다음:
- ChatGPT가 `00_plan/progress_log.md`를 갱신한다.
- 다음 공부 시작 전에는 다시 `git pull`부터 한다.

Role split:
- User: Day 폴더 산출물 정리 + commit/push
- ChatGPT: `00_plan/progress_log.md` 갱신

Default pwn/reversing study routine:
- 매 문제마다 입력 지점 확인 → data flow 추적 → 검증/제한 위치 확인 → 위험 sink 확인 → root cause 설명 → 소스 없이 찾는 방법 설명 → exploitability 판단 → exploit 작성/검증 루틴을 기본으로 적용한다.
- 사용자가 별도로 생략하라고 하지 않는 한, write-up 또는 복기 단계에 root cause와 discovery 관점을 포함한다.

---

## Current Pointer

- Last completed: Day064
- Current focus: UAF 1 완료, dangling pointer와 객체 lifetime 정리 완료
- Next task: Day065 UAF 2 진행
- Repo rule: 각 Day 폴더 안에 그날의 바이너리, 소스, exploit, write-up, 실행 결과를 넣는다.

---

## Recent Daily Log

### Day060
- Topic: Month2 portfolio summary + reproduction verification
- Status: done
- Result: `month2_portfolio_readme.md`를 작성해 Day41~Day59 핵심 기법, 대표 실습, 검증 체크리스트, failure cases, CS review, next month plan을 정리했다. Day44, Day46, Day50 exploit을 clean run으로 재현해 Verified 처리했다.
- Files: Day040-100/Day060/month2_portfolio_readme.md
- Problems: 포트폴리오 정리에서는 성공 케이스뿐 아니라 실패 원인과 검증 명령을 남겨야 한다.
- Next: Day061

### Day061
- Topic: Heap intro: malloc/free, UAF, double free, heap overflow, tcache
- Status: done
- Result: stack pointer 변수와 heap chunk의 차이, `free()`가 포인터를 NULL로 만들지 않는 점, dangling pointer, UAF read/write, alias pointer, double free, heap overflow, metadata, tcache LIFO 재사용, tcache poisoning 기본 흐름을 정리했다.
- Files: Day040-100/Day061/heap_intro_notes.md
- Problems: `free(p)`는 heap chunk를 allocator에 반납할 뿐 `p` 값을 자동으로 NULL로 바꾸지 않는다. `p=NULL`은 alias pointer까지 해결하지 못한다.
- Next: Day062

### Day062
- Topic: Heap metadata: size/flag 관찰
- Status: done
- Result: `malloc(0x30)`의 실제 chunk size가 `0x40`이고 `size|flags=0x41`, `malloc(0x80)`의 실제 chunk size가 `0x90`이고 `size|flags=0x91`임을 gdb로 확인했다. user pointer 기준 `p-0x10=prev_size`, `p-0x8=size|flags` 구조와 `PREV_INUSE` 의미를 정리했다. `free(a)` 후 같은 size class의 `malloc(0x30)`에서 `d==a`가 되는 tcache 재사용과 UAF write/read 가능성을 확인했다.
- Files: Day040-100/Day062/day62_heap.md
- Problems: `PREV_INUSE`는 현재 chunk가 아니라 이전 chunk 상태를 나타낸다. `prev_size`는 `PREV_INUSE=0`일 때만 의미 있다. 작은 chunk가 tcache에 들어가면 즉시 coalescing되지 않아 다음 chunk의 `PREV_INUSE`가 바로 0으로 바뀌지 않을 수 있다.
- Next: Day063

### Day063
- Topic: tcache bin/free list 검증
- Status: done
- Result: 같은 size class chunk `a,b,c`를 `free(a); free(b); free(c);` 순서로 해제했을 때 tcache list가 `c -> b -> a -> NULL`이 되는 LIFO 구조를 gdb `tcachebins`와 raw memory로 교차검증했다. 이후 `malloc(0x30)` 반복에서 `d==c`, `e==b`, `f==a` 순서로 pop되는 것을 확인했다. safe-linking 때문에 raw next 값은 encoded되어 저장되며 `real_next = encoded_next ^ (chunk_addr >> 12)`로 복원해야 함을 확인했다.
- Files: Day040-100/Day063/day63_heap.md
- Problems: safe-linking decode 시 next로 가리켜지는 chunk 주소가 아니라 encoded next가 저장된 현재 chunk 주소를 기준으로 `>> 12` 해야 한다. malloc 후에도 user data가 초기화되지 않아 stale tcache metadata가 남을 수 있고, 이를 잘못 해석하면 allocated chunk가 아직 tcache 안에 있다고 착각할 수 있다.
- Next: Day064

### Day064
- Topic: UAF 1 - dangling pointer bug 재현
- Status: done
- Result: `a`를 `free(a); a=NULL;` 처리해도 `alias=a`로 복사해둔 포인터에는 old heap 주소가 남는 것을 확인했다. 같은 size class로 `b`를 다시 할당하면 allocator가 같은 chunk를 재사용해 `alias==b`가 되고, `alias->name` write가 `b->name`을 덮는 UAF write primitive를 재현했다. 주소 관점과 객체 lifetime 관점을 분리해 정리했다.
- Files: Day040-100/Day064/day64_heap.md
- Problems: `a=NULL`은 alias pointer를 해결하지 못한다. 다른 size class로 재할당하면 `alias==b`가 안 되어 즉시 재현은 실패할 수 있지만, dangling pointer가 남아 있으므로 UAF bug 자체가 사라지는 것은 아니다. 이번 primitive는 double free나 heap overflow가 아니라 stale alias pointer 기반 UAF write다.
- Next: Day065 UAF 2

---

## Update Template

### DayXXX
- Topic:
- Status:
- Result:
- Files:
- Problems:
- Next:
