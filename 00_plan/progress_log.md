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

- Last completed: Day068
- Current focus: Double free 2 완료, tcache free list 순서 조작과 malloc 반환 순서 검증 완료
- Next task: Day069 진행
- Repo rule: 각 Day 폴더 안에 그날의 바이너리, 소스, exploit, write-up, 실행 결과를 넣는다.

---

## Recent Daily Log

### Day064
- Topic: UAF 1 - dangling pointer bug 재현
- Status: done
- Result: `a`를 `free(a); a=NULL;` 처리해도 `alias=a`로 복사해둔 포인터에는 old heap 주소가 남는 것을 확인했다. 같은 size class로 `b`를 다시 할당하면 allocator가 같은 chunk를 재사용해 `alias==b`가 되고, `alias->name` write가 `b->name`을 덮는 UAF write primitive를 재현했다. 주소 관점과 객체 lifetime 관점을 분리해 정리했다.
- Files: Day040-100/Day064/day64_heap.md
- Problems: `a=NULL`은 alias pointer를 해결하지 못한다. 다른 size class로 재할당하면 `alias==b`가 안 되어 즉시 재현은 실패할 수 있지만, dangling pointer가 남아 있으므로 UAF bug 자체가 사라지는 것은 아니다. 이번 primitive는 double free나 heap overflow가 아니라 stale alias pointer 기반 UAF write다.
- Next: Day065

### Day065
- Topic: UAF 2 - leak primitive 구성
- Status: done
- Result: `Obj *alias = a`로 복사해둔 dangling pointer가 같은 size class 재할당으로 새 객체 `b`와 같은 chunk를 가리키는 상황을 만들었다. `b->ptr = secret` 이후 `alias->ptr`을 읽으면 `alias->ptr == b->ptr == secret`이 되어 secret heap 주소와 문자열을 leak할 수 있음을 확인했다. gdb에서 `alias == b`, `alias->ptr == b->ptr`, `x/6gx alias == x/6gx b`로 raw memory를 교차검증했다.
- Files: Day040-100/Day065/day65_heap.md, notes/cs_fundamentals.md
- Problems: 다른 size class로 `b`를 재할당하면 같은 chunk가 재사용되지 않아 `alias==b`가 성립하지 않고 leak 재현이 실패할 수 있다. 단, dangling pointer가 남아 있으므로 UAF bug 자체가 사라지는 것은 아니다. UAF read는 방법이고, 이번 결과는 UAF 기반 heap leak primitive로 정리한다.
- Next: Day066

### Day066
- Topic: UAF 3 - overwrite로 흐름 변화 만들기
- Status: done
- Result: `User *alias = a`로 복사해둔 dangling pointer가 같은 size class 재할당으로 새 객체 `b`와 같은 chunk를 가리키는 상황을 만들었다. `alias->is_admin = 1`을 수행하자 실제로 `b->is_admin`이 1로 바뀌고, `check(b)`의 분기가 `access denied`에서 `admin access granted`로 바뀌는 것을 확인했다. gdb에서 첫 번째 `check(b)`의 `is_admin=0`, overwrite 이후 두 번째 `check(b)`의 `is_admin=1`, raw memory의 `b+0x20=0x1`을 교차검증했다.
- Files: Day040-100/Day066/day66_heap.md, notes/cs_fundamentals.md
- Problems: 다른 size class로 `b`를 재할당하면 freed `a` chunk가 `b`로 재사용되지 않아 `alias==b`가 성립하지 않고 `alias->is_admin` write가 `b->is_admin`을 바꾸지 못한다. 오늘 실습은 type confusion이 아니라 같은 타입 객체의 분기 조건 field를 덮는 UAF overwrite 사례다. type confusion exploit은 같은 주소 재사용과 target offset까지 도달 가능한 write가 함께 필요하다.
- Next: Day067

### Day067
- Topic: Double free 1 - tcache duplicate 상태 만들기
- Status: done
- Result: 단순 `free(a); free(a);`에서는 glibc가 `free(): double free detected in tcache 2`로 abort하는 것을 확인했다. 첫 번째 `free(a)` 이후 raw memory에서 `a+0x00`의 encoded next와 `a+0x08`의 tcache key를 확인했고, `tcachebins`에서 `tcache[0x40] → a → NULL` 상태를 검증했다. 이후 UAF write로 `((uintptr_t *)a)[1] = 0`을 수행해 `a+0x08`의 key를 corruption한 뒤 두 번째 `free(a)`가 통과했고, `malloc(0x30)` 두 번에서 `p == q`가 되어 같은 chunk가 중복 반환되는 것을 확인했다.
- Files: Day040-100/Day067/day67_heap.md
- Problems: key를 망가뜨리지 않고 바로 두 번째 `free(a)`를 호출하면 glibc가 double free를 감지하고 abort한다. double free가 우회되면 같은 chunk가 tcache에 중복 등록되어 active object처럼 보이는 두 포인터가 같은 heap 주소를 가리키는 alias 상태가 된다.
- Next: Day068

### Day068
- Topic: Double free 2 - allocation 순서 조작
- Status: done
- Result: `free(a); free(b); ((uintptr_t *)a)[1] = 0; free(a);` 흐름으로 `a+0x08`의 tcache key를 corruption해 double-free detection을 우회했다. 그 결과 tcache 구조가 `tcache[0x40] -> a -> b -> a -> ...` 형태로 오염되었고, 이후 `malloc(0x30)` 세 번에서 `p=a`, `q=b`, `r=a`가 되어 `p == r`, `p != q`를 확인했다. gdb에서 `tcachebins`, raw memory의 encoded next, malloc 이후 포인터 값을 교차검증했다.
- Files: Day040-100/Day068/day68_heap.md, notes/cs_fundamentals.md
- Problems: key corruption 없이 두 번째 `free(a)`를 호출하면 `a+0x08`의 tcache key가 남아 있어 glibc가 `free(): double free detected in tcache 2`로 abort한다. tcache duplicate check는 freed chunk 내부 key를 이용해 중복 free 가능성을 의심하며, UAF write로 key를 망가뜨리면 free list 순서가 오염될 수 있다.
- Next: Day069

---

## Update Template

### DayXXX
- Topic:
- Status:
- Result:
- Files:
- Problems:
- Next:
