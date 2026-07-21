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
- 사용자가 "시작하자" 또는 특정 Day 시작을 요청하면 GitHub progress log와 최신 `보안 계획표.xlsx`의 해당 Day를 함께 확인한다.
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

Day080 additional exam plan:
- 엑셀에 원래 계획된 Day080 미니시험은 그대로 진행한다.
- 추가로 GitHub Day001~Day079 write-up 전체를 확인해 보안 필기시험 약 30문항을 만든다.
- 사용자가 Day080에 제공하는 별도 CS 정리 파일을 바탕으로 CS 필기시험 20~30문항을 추가한다.
- 추가 필기시험은 개념, 메모리 상태, gdb/raw memory 해석, 실패 원인, exploit 흐름, 틀린 설명 교정 중심으로 구성한다.

---

## Current Pointer

- Last completed: Day078
- Current focus: Unsorted bin 2 완료. large chunk를 free한 뒤 user area의 `fd/bk`가 `0x7ffff7e1ace0`인 libc `rw-p` 주소를 가리키는 것을 raw memory로 확인했다. `leak - 0x21ace0 = 0x7ffff7c00000`으로 libc base를 계산하고 `vmmap`의 file offset 0 mapping과 일치함을 검증했다. 잘못된 offset `0x219c80`을 사용하면 base가 `0x1060`만큼 틀리고 page alignment도 깨지는 실패 케이스를 재현했다. CS에서는 libc 내부 전역구조가 free chunk의 `fd/bk`를 통해 노출되는 이유, UAF/OOB read 필요성, 재할당 후 stale metadata를 신뢰하면 안 되는 이유, 다른 libc leak과의 교차검증을 정리했다.
- Next task: Day079 진행 — Unsorted bin 3: exploit 연결. unsorted leak을 ret2libc/ROP 계획에 연결하고, leak→base→target 주소 계산 흐름과 실패 케이스를 검증한다.
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

### Day069
- Topic: Double free 3 - exploit/write-up
- Status: done
- Result: double free로 `tcache[0x40] -> a -> b -> a -> NULL` 상태를 만든 뒤 `malloc(0x30)` 세 번에서 `p=a`, `q=b`, `r=a`를 확인했다. `p == r`, `p != q` 상태에서 `p`의 user area를 `A`로 덮자 `r`에서도 같은 raw memory가 보였고, 별도 chunk인 `q`에는 영향이 없었다. 이를 통해 overlapping allocation 기반 cross-object overwrite 가능성을 검증했다.
- Files: Day040-100/Day069/day69_heap.md, notes/cs_fundamentals.md
- Problems: 세 번째 요청을 `malloc(0x39)`로 바꾸면 `0x50` size class를 사용해 `tcache[0x40]`의 중복 `a`를 소비하지 않는다. 그 결과 `p != r`이 되고, `tcache[0x40]`에 `a`가 남아 overlapping allocation이 실패한다.
- Next: Day070

### Day070
- Topic: Heap write-up 1 - UAF와 double free 비교
- Status: done
- Result: UAF는 lifetime이 끝난 객체를 stale pointer로 사용하는 버그이고, double free는 같은 allocation을 allocator에 두 번 반환하는 버그로 구분했다. 재할당 전 UAF write로 freed chunk의 tcache key를 corruption해 double-free detection을 우회하고, 중복 등록된 chunk를 통해 overlapping allocation까지 이어지는 exploit chain을 정리했다. heap 다이어그램에서는 valid pointer와 stale pointer, owner가 program인지 allocator인지, 같은 physical chunk의 중복 entry 여부를 명시하는 방식으로 정리했다.
- Files: Day040-100/Day070/day70_heap.md, notes/cs_fundamentals.md
- Problems: 다른 size class 할당으로 즉시 chunk 재사용이 실패해도 dangling pointer 자체는 남는다. 반대로 tcache key를 손상시키지 않으면 두 번째 free에서 abort하지만, 이는 버그 부재가 아니라 glibc 방어가 해당 실행에서 성공한 것이다.
- Next: Day071

### Day071
- Topic: Tcache poisoning 1 - 개념/toy
- Status: done
- Result: `tcache[0x40] -> a -> b` 상태에서 UAF write로 `a`의 user area 첫 8바이트인 encoded `next`를 `target ^ (a >> 12)`로 덮어 `a -> target` 체인을 만들었다. 같은 size class `malloc` 두 번에서 `p=a`, `q=target`을 확인했고, `q`를 통한 write가 전역 aligned target의 raw memory에 반영되어 target address allocation과 targeted write primitive를 검증했다. poisoning으로 기존 `b`는 메모리에는 남지만 free-list에서 도달 불가능한 orphaned freed chunk가 됨을 정리했다.
- Files: Day040-100/Day071/day71_heap.md, notes/cs_fundamentals.md
- Problems: encoded value 대신 raw target 주소를 `a->next`에 쓰면 safe-linking 복호화 시 `target ^ (a >> 12)`가 next로 해석되어 target 반환이 실패하거나 abort한다. 또한 target 주소가 16바이트 정렬을 만족하지 않으면 `unaligned tcache chunk detected`로 실패할 수 있다. `q == target`만으로는 allocation primitive이며, 실제 write 경로와 주소·offset·값·크기 통제가 있어야 stronger write primitive로 확장된다.
- Next: Day072

### Day072
- Topic: Tcache poisoning 2 - controlled allocation
- Status: done
- Result: `goal = target + 0x20`으로 설정하고 `a->next`에 `goal ^ (a >> 12)`를 저장해 `tcache[0x40] -> a -> goal` 상태를 만들었다. fake entry인 `goal`의 encoded NULL을 `goal >> 12`로 구성한 뒤 같은 size class `malloc` 두 번에서 `p == a`, `q == goal`을 확인했다. 이후 `memset(q, 'A', 8)`이 `goal` raw memory를 `0x4141414141414141`로 바꾸는 것을 검증했다. CS에서는 controlled allocation과 arbitrary allocation, allocation primitive와 write primitive, 제약 있는 arbitrary primitive 표현을 구분했다.
- Files: Day040-100/Day072/day72_heap.md, notes/cs_fundamentals.md
- Problems: `goal = target + 0x18`로 바꾸면 16바이트 정렬이 깨져 `malloc(): unaligned tcache chunk detected`로 실패할 수 있다. 복습 필기 30문항에서는 stack/ROP/PIE/ret2libc와 UAF/tcache를 점검했고, saved RIP 용어, NULL canary와 문자열 함수, `strip()`의 whitespace 제거, root cause와 exploit 결과 구분, controlled/arbitrary allocation 표현을 보완했다.
- Next: Day073

### Day073
- Topic: Tcache poisoning 3 - control data overwrite
- Status: done
- Result: `goal = &target.callback`으로 설정하고 safe-linking된 `a->next`를 조작해 `tcache[0x40] -> a -> &target.callback` 상태를 만들었다. 같은 size class `malloc` 두 번에서 `p == a`, `q == &target.callback`을 확인한 뒤 `q`를 통해 callback 값을 `safe` 주소에서 `win` 주소로 덮었다. raw memory와 심볼 값을 비교하고 `win` breakpoint hit 및 `WIN` 출력을 통해 control data overwrite와 control-flow hijack을 검증했다. CS에서는 함수 포인터가 함수 시작 주소를 저장하고 간접 호출 시 RIP의 이동 대상으로 사용된다는 점, hook overwrite 위험, PIE와 함수 주소 계산, 함수 시그니처 호환 조건을 정리했다.
- Files: Day040-100/Day073/day73_heap.md, notes/cs_fundamentals.md
- Problems: callback에 `0x4141414141414141`을 기록하면 overwrite 자체는 성공하지만 간접 호출 시 유효하지 않은 실행 주소로 분기해 `Segmentation fault`가 발생한다. 따라서 control data overwrite 성공과 control-flow hijack 성공을 분리해 검증해야 한다.
- Next: Day074

### Day074
- Topic: Heap leak 1 - heap pointer disclosure
- Status: done
- Result: `holder->ptr = target`인 toy에서 프로그램 출력 leak, `holder->ptr`, `target`, `x/gx &holder->ptr`가 모두 같은 heap 주소임을 확인했다. `vmmap`, `heap chunks`, raw memory를 교차검증하고 첫 실행에서 `offset = leak - heap_base = 0x310`을 구했다. ASLR로 절대주소가 바뀐 새 실행에서도 `new_leak - 0x310`이 실제 `[heap]` 시작 주소와 일치해 고정 상대 offset을 이용한 heap base 복원을 검증했다. CS에서는 heap pointer disclosure가 ASLR 우회용 보조 primitive이며 별도의 read/write primitive와 결합돼야 한다는 점, glibc chunk의 `prev_size`, `size|flags`, `PREV_INUSE`, `IS_MMAPPED`, `NON_MAIN_ARENA`를 정리했다.
- Files: Day040-100/Day074/day74_heap.md, notes/cs_fundamentals.md
- Problems: `target` 앞에 `malloc(0x30)`을 하나 추가하자 target offset이 `0x310`에서 `0x350`으로 변했다. 요청 크기는 `0x30`이지만 실제 chunk 크기는 metadata와 정렬을 포함해 `0x40`이므로 target이 정확히 `0x40` 밀렸다. 기존 offset을 계속 사용하면 계산된 heap base가 실제 base보다 `0x40` 크게 나와 실패한다.
- Next: Day075

### Day075
- Topic: Heap leak 2 - libc pointer disclosure
- Status: done
- Result: `Handler { name[32], callback }` 객체의 `callback`에 `puts`를 저장한 뒤 해제하고 같은 크기의 객체를 재할당했다. 새 객체에서는 `name` 8바이트만 갱신되어 `callback`이 초기화되지 않았고, `fresh->callback`, `x/gx &fresh->callback`, `puts`가 모두 `0x7ffff7c80e50`으로 일치했다. 재사용된 객체의 부분 초기화가 stale libc pointer disclosure로 이어짐을 확인했다.
- Files: Day040-100/Day075/day75.c, Day040-100/Day075/write_up.txt
- Problems: 객체 전체를 `calloc`, `memset`, 명시적 `NULL` 대입으로 초기화하면 stale pointer가 제거된다. 같은 size class chunk가 재사용되지 않으면 동일한 leak이 즉시 재현되지 않을 수 있다.
- Next: Day076

### Day076
- Topic: Heap leak 3 - libc base 계산
- Status: done
- Result: `puts leak = 0x7ffff7c80e50`, `puts offset = 0x80e50`을 사용해 `libc base = 0x7ffff7c00000`을 계산했다. `vmmap`의 file offset 0 libc mapping과 계산값이 일치함을 확인했고, `r-xp` mapping은 base보다 `0x28000` 뒤에서 시작한다는 점을 구분했다.
- Files: Day040-100/Day076/write_up.txt
- Problems: `r-xp` 시작점을 libc base로 사용하면 이후 주소가 `0x28000`만큼 틀어진다. 다른 libc build의 offset이나 함수 내부 주소를 symbol 시작 offset으로 잘못 빼도 base와 `system` 주소가 잘못 계산된다.
- Next: Day077

### Day077
- Topic: Unsorted bin 1 - chunk 이동 관찰
- Status: done
- Result: `malloc(0x500)` victim 뒤에 allocated guard를 배치하고 victim을 free하자 unsorted bin에 들어갔으며, user area의 `fd`와 `bk`가 모두 `0x7ffff7e1ace0`인 libc `rw-p` 주소를 가리켰다. chunk가 하나인 원형 이중 연결 리스트에서는 양쪽 pointer가 모두 bin head를 가리킴을 확인했다. guard를 제거하면 victim이 top과 병합되어 size가 `0x20d71`로 커지고 유효한 unsorted `fd/bk`가 생성되지 않았다. CS에서는 `PREV_INUSE`, `prev_size`, boundary tag, forward/backward consolidation, tcache와 일반 free 경로 차이를 정리했다.
- Files: Day040-100/Day076/day76.c, Day040-100/Day076/day77.c, Day040-100/Day077/write_up.txt
- Problems: guard가 없는 경우 user area에 기존 `A` 값이 남아 있어도 이는 stale data일 뿐 unsorted metadata가 아니다. 초기에는 재컴파일하지 않은 바이너리를 실행해 guard chunk의 `0x31` header가 보였고, 재컴파일 후 top consolidation 상태를 다시 검증했다. Day077 실습 소스가 현재 Day076 폴더에 있으므로 다음 정리 시 Day077 폴더로 이동할 필요가 있다.
- Next: Day078

### Day078
- Topic: Unsorted bin 2 - main_arena leak
- Status: done
- Result: `malloc(0x500)` victim 뒤에 guard chunk를 둔 뒤 victim을 free하고 raw memory를 확인했다. `fd`와 `bk`가 모두 `0x7ffff7e1ace0`으로 같았고, 해당 값이 libc `rw-p` mapping에 속함을 검증했다. `0x7ffff7e1ace0 - 0x21ace0 = 0x7ffff7c00000`으로 libc base를 복원했으며, 계산값이 `vmmap`의 libc file offset 0 mapping 시작점과 정확히 일치했다. CS에서는 libc 내부 bin head 주소가 free chunk의 user area에 기록되고 UAF/OOB read로 leak되는 구조, 같은 libc build에서 offset이 고정되는 이유, 다른 leak과의 교차검증을 정리했다.
- Files: Day040-100/Day078/day78.c, Day040-100/Day078/day78, Day040-100/Day078/.gdb_history, Day040-100/Day078/write_up.txt
- Problems: 잘못된 offset `0x219c80`을 사용하면 `wrong base = 0x7ffff7c01060`이 되어 실제 base보다 `0x1060` 크게 계산되고 page alignment 및 `vmmap` 검증에 실패한다. 재할당된 chunk에 남은 값은 현재 `fd/bk`가 아니라 일부가 덮인 stale data일 수 있으므로 주소 형태만 보고 신뢰하면 안 된다.
- Next: Day079

---

## Update Template

### DayXXX
- Topic:
- Status:
- Result:
- Files:
- Problems:
- Next:
