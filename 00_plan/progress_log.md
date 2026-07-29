# Progress Log

이 파일은 새 채팅에서도 바로 이어가기 위한 기준 파일이다.
매일 공부가 끝나면 아래 포인터와 운영 규칙을 기준으로 갱신한다.

- Last completed: 마지막으로 완료한 Day
- Current focus: 방금 완료한 핵심 내용과 검증
- Next task: 다음에 바로 시작할 작업

---

## Operating Rule

공부 시작 전:
- GitHub repo와 `00_plan/progress_log.md`를 확인한다.
- 사용자가 시작을 요청하면 최신 `보안 계획표.xlsx`의 해당 Day도 함께 확인한다.
- 사용자는 먼저 아래 명령을 실행한다.

```bash
cd /mnt/d/security-roadmap
git pull
```

공부 끝난 후:
- User: 해당 Day 폴더에 바이너리, 소스, exploit, write-up, 실행 결과를 정리하고 commit/push한다.
- ChatGPT: 사용자 commit을 확인한 뒤 `00_plan/progress_log.md`를 갱신한다.
- 다음 공부 시작 전에는 다시 `git pull`부터 한다.

Default pwn/reversing routine:
- 입력 지점 확인 → data flow → 검증/제한 → 위험 sink → root cause → 소스 없이 찾는 방법 → exploitability → exploit 작성/검증 순서로 진행한다.
- 성공 1회로 완료하지 않는다. GDB/raw memory 검증, 실패 케이스, 짧은 write-up까지 확인한다.
- 사용자가 먼저 분석과 계획을 말하고, ChatGPT는 정답을 미리 전부 주지 않는다.

Daily review rule:
- 본 학습과 CS 종료 후 10~20분 동안 오래됐거나 적게 다룬 주제를 복습한다.
- GOT overwrite, shellcode처럼 실습 횟수가 적은 항목을 우선하고 오래된 순서와 랜덤 선정을 섞는다.
- 질문 3개 또는 짧은 실전 문제 1개로 진행한다.
- 사용자 답변 뒤 보충은 틀린 핵심과 빠진 핵심만 3~5줄로 설명한다.

---

## Current Pointer

- Last completed: Day083
- Current focus: Day084 실습과 CS 학습은 진행했다. 잘못된 free 순서에서 tcache LIFO 때문에 기대한 chunk 대신 마지막으로 free한 chunk가 반환되는 현상, 동일 chunk 재해제 시 glibc의 tcache 중복 검사로 `SIGABRT`가 발생하는 현상, `puts` leak에 잘못된 symbol offset을 적용해 `wrong_system == puts_leak`이 되어 `puts("/bin/sh")`만 실행되는 주소 계산 실패를 분류했다. GDB의 `tcachebins`, raw chunk metadata, libc mapping/page alignment를 검증 기준으로 정리했고, CS에서는 `SIGABRT`·`SIGSEGV`·정상 종료 오동작을 구분했다. GOT overwrite와 shellcode 인자 배치도 복습했다. 사용자 commit `12367541f1268d0a621e9b6d8631cfbf7ae7396a`은 확인했지만 write-up에 수정할 항목이 남아 Day084 완료 처리는 보류한다.
- Next task: Day084 write-up을 수정하고 다시 commit한다. 실패 1의 `tcachebins`는 `b → a → NULL`, raw memory는 `b->next`가 `a`를 encode한 값으로 고친다. 실패 2의 첫 free 이후 `a+0x00 = a >> 12`(encoded NULL), `a+0x08 = tcache key`로 고치며, 일반 double-free abort 상태와 key 변조 후 `a → a` 우회 상태를 분리한다. `page alignment`는 하위 12비트/마지막 3 hex digit가 0임을 명시하고, `CS: 실패 로그 분류`를 commit에 추가한다. 수정 확인 후 Day085 Heap portfolio checkpoint로 진행한다.
- Repo rule: 각 Day 폴더 안에 그날의 바이너리, 소스, exploit, write-up, 실행 결과를 넣는다.

---

## Recent Daily Log

### Day079
- Topic: Unsorted bin 3 - exploit 연결
- Status: done
- Result: 메뉴형 UAF에서 unsorted-bin `fd/bk`를 leak해 libc base를 계산하고, 별도 BOF를 `ret → pop rdi → /bin/sh → system`으로 연결해 shell을 획득했다.
- Problems: raw 16바이트를 `fd=data[:8]`, `bk=data[8:16]`으로 분리해야 하며, 이번 체인은 tcache poisoning을 사용하지 않았다.
- Next: Day080

### Day080
- Topic: Heap mini-exam + Day001~Day079 보안 필기시험
- Status: done
- Result: unsorted-bin leak과 Safe-Linking mask leak, tcache poisoning, callback overwrite를 한 프로세스에서 결합해 `system("/bin/sh")`을 실행했다. 추가 필기시험 30문항과 오답 복습도 완료했다.
- Problems: tcache count가 1이면 poisoned target까지 반환되지 않으며, tail/NULL entry에서 mask를 구해야 한다. raw leak은 고정 길이 수신이 안전하다.
- Next: Day081

### Day081
- Topic: Hook overwrite / modern glibc 대체 흐름
- Status: done
- Result: glibc 2.35에서 `__free_hook` 심볼은 존재하고 writable했지만 `free()` 실행 경로가 사용하지 않음을 확인했다. 대신 writable `.data` callback을 `puts`에서 `system`으로 덮어 shell을 획득했다.
- Problems: target은 writable뿐 아니라 addressable, triggerable, argument-compatible해야 한다. Full RELRO는 일반 `.data` callback을 보호하지 않는다.
- Next: Day082

### Day082
- Topic: Heap → ROP 전환 / heap fake stack pivot
- Status: done
- Result: callback을 `leave; ret`으로 덮고 heap 객체 내부 fake stack으로 pivot해 `system("/bin/sh") → exit(0)` ROP를 실행했다.
- Problems: fake stack을 heap mapping 시작 근처에 두면 `system()`의 하향 stack 확장 공간이 부족해 실패했다. 큰 region의 높은 offset으로 옮겨 해결했다.
- Next: Day083

### Day083
- Topic: Heap 복합 문제 1 - UAF stale libc leak + callback overwrite
- Status: done
- Result: `add → delete → show → edit → trigger` 흐름으로 dangling pointer 기반 raw read/write를 구성했다. `show`에서 `obj+0x20`의 stale `puts`를 leak하고 libc base와 `system`을 계산했다. `edit` payload를 `b"/bin/sh\x00".ljust(0x20, b"\x00") + p64(system)`으로 구성해 callback을 덮고 `trigger`에서 `system(data)`를 호출해 shell을 획득했다. GDB로 slot 주소, chunk raw memory, callback 값과 libc mapping을 확인했다.
- Files: Day040-100/Day083/.gdb_history, Day040-100/Day083/Makefile, Day040-100/Day083/day83, Day040-100/Day083/day83.c, Day040-100/Day083/exploit.py, Day040-100/Day083/write_up.txt
- Problems: `edit`를 `show`보다 먼저 실행하면 stale `puts`가 덮여 libc leak을 잃는다. freed chunk 전체 edit는 encoded next와 tcache key도 손상시키므로 이후 같은 size class에서 `malloc/free`를 다시 수행하면 allocator가 오염된 metadata를 해석해 실패할 수 있다. 이번 exploit은 edit 직후 allocator를 다시 사용하지 않고 dangling pointer의 callback을 직접 trigger했다.
- Next: Day084

### Day084
- Topic: Heap 실패 케이스 day
- Status: correction needed
- Result: 잘못된 free 순서, double-free abort, 잘못된 libc base를 각각 재현·분류했다. tcache LIFO, tcache key 기반 중복 검사와 bin 순회, 잘못된 offset 상쇄로 `wrong_system == puts_leak`이 되는 원리를 설명했다. CS에서는 `SIGABRT`는 allocator가 의도적으로 종료한 경우, `SIGSEGV`는 잘못된 주소 접근/점프, 정상 종료 오동작은 유효하지만 틀린 함수가 실행된 경우로 분류했다. GOT overwrite와 shellcode stack 배치를 복습했다.
- Files: Day040-100/Day084/day84, Day040-100/Day084/day84.c, Day040-100/Day084/write_up.txt
- Problems: commit의 실패 1 tcache/raw-memory 방향이 반대로 기록됐고, 실패 2의 `a+0x00`과 `a+0x08` 설명이 뒤바뀌었다. 기본 abort 상태와 key 변조 우회 후 self-loop 상태가 혼재되어 있으며 CS 실패 로그 분류가 write-up에 누락됐다.
- Next: Day084 write-up correction → Day085

---

## Historical Log

- Day064~Day078의 상세 기록은 각 Day 폴더와 이전 `progress_log.md` commit history에 보존되어 있다.
- Day080 추가 시험 계획과 이전 포인터 변경 이력도 Git history에서 확인한다.

---

## Update Template

### DayXXX
- Topic:
- Status:
- Result:
- Files:
- Problems:
- Next:
