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

- Last completed: Day086
- Current focus: Day087과 Day088의 핵심 정적 분석 및 정상 실행까지 진행했다. Day087에서는 strings/XREF/caller 흐름으로 main과 실제 성공 함수, caller가 없는 `FUN_00101510` decoy를 구분하고 `route_87` / `xref_guides_flow`를 복원했으며 GDB에서 `check_alias()` 직후 `EAX=1`을 확인했다. Day088에서는 공통 context의 구조를 `magic@0x00`, `version@0x04`, `flags@0x06`, `handle[16]@0x08`, `phrase[24]@0x18`, `key[8]@0x30`, `score@0x38`, `digest@0x3c`로 복원하고 `recover_the_type` phrase와 성공 출력을 확인했다. 사용자 commit `d78c581e6498d2c365ba08f3bde108fc1670ef6a`은 확인했으나, Day087 실패 경로 재현과 Day088의 `day88_rev.md`, type 적용 전후 근거, GDB/objdump 교차검증, 실패 경로 재현이 저장소에 아직 없다.
- Next task: Day087 실패 경로 1개와 Day088 필수 근거를 마무리해 추가 commit/push한다. Day088에서는 Ghidra에 custom `AnalysisContext` 타입을 실제 적용해 raw offset 표현이 필드 접근으로 바뀐 전후 화면을 남기고, `score` 또는 `digest`를 GDB/objdump로 교차검증하며 잘못된 handle/phrase 실패 경로를 기록한다. 이 검증이 끝난 뒤 Day089로 이동한다.
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
- Status: done
- Result: 잘못된 free 순서, double-free abort, 잘못된 libc base를 각각 재현·분류했다. `free(a); free(b);` 이후 상태가 `b → a → NULL`임을 정정하고, raw metadata에서 `b+0x00`은 encoded `a`, `a+0x00`은 encoded NULL인 `a >> 12`, `a+0x08`은 tcache key임을 기록했다. 기본 재해제는 `free(): double free detected in tcache 2`와 `SIGABRT`를 발생시키며, key를 변조한 추가 실험에서는 count 2와 `a → a` self-loop가 만들어짐을 분리해 설명했다. 잘못된 libc offset에서는 `wrong_system == puts_leak`이 되어 `puts("/bin/sh")`만 실행되는 원리도 확인했다.
- Files: Day040-100/Day084/day84, Day040-100/Day084/day84.c, Day040-100/Day084/write_up.txt
- Problems: allocator abort 상태와 metadata 변조 우회 상태를 혼동하지 않아야 하며, tcache freelist 방향과 raw encoded pointer를 별도로 검증해야 한다.
- Next: Day085

### Day085
- Topic: Heap Exploitation Capstone / Heap 포트폴리오 checkpoint
- Status: done
- Result: small UAF raw read에서 단일 tcache entry의 encoded NULL을 Safe-Linking mask로 사용하고, 객체의 stale `self`와 `default_callback` 필드에서 heap/PIE 정보를 확인했다. large UAF는 guard allocation으로 top consolidation을 막아 unsorted-bin `fd`를 leak하고 libc base와 `system`을 계산했다. small tcache count를 2로 구성한 뒤 `encoded_target = (control_region+0x4000) ^ mask`로 poisoning해 writable `.bss` 주소를 두 번째 `malloc(0x60)` 반환값으로 얻었다. 해당 영역에 dummy RBP, 정렬용 `ret`, `pop rdi; ret`, `"/bin/sh"`, `system`, `leave; ret` callback을 배치해 stack pivot과 shell 실행에 성공했고 `id`, `whoami` 명령 결과까지 확인했다.
- Files: Day040-100/Day085/.gdb_history, Day040-100/Day085/Makefile, Day040-100/Day085/day85, Day040-100/Day085/day85.c, Day040-100/Day085/exploit.py, Day040-100/Day085/write_up.txt
- Problems: tcache count가 1이면 poisoned target이 반환되지 않는다. target을 평문으로 기록하면 Safe-Linking 검증/디코딩 과정에서 실패하며, 정렬용 `ret`을 제거하면 `system()` 진입 시 ABI 정렬 문제로 실패할 수 있다. Full RELRO에서는 GOT 대신 writable하고 trigger 가능한 control object를 선정해야 한다.
- Next: Day086

### Day086
- Topic: Reversing — Ghidra 1: import와 함수 목록 파악
- Status: done
- Result: stripped PIE의 entry에서 `__libc_start_main` 첫 번째 인자를 추적해 main offset `0x15d7`을 찾았다. 문자열, xref, decompiler와 Listing을 이용해 `read_token`, `check_format`, `transform_token`, `check_transformed`, `calculate_token_hash`, 성공/실패 출력 함수의 역할을 복원했다. 변환식 `out[i] = (in[i] ^ key[i % 5]) + i`, key `13 37 21 5a 0c`, 기대 배열을 바탕으로 `xref_and_strings`를 역산하고 rolling hash `0xf3d270b4`까지 일치함을 확인했다. GDB에서 `$rbp-0x20`의 변환 결과와 `$base+0x2020`의 기대 배열이 동일하고 래퍼 반환값이 `EAX=1`임을 확인했으며 실제 실행도 성공했다.
- Files: Day040-100/Day086/.gdb_history, Day040-100/Day086/README.txt, Day040-100/Day086/day86_challenge, Day040-100/Day086/write_up.txt
- Problems: Ghidra 주소 `0x001014c5`에서 image base를 빼 RVA `0x14c5`를 구한 뒤 runtime PIE base를 더해야 한다. Ghidra 변수명 `local_28`과 실제 스택 offset `[rbp-0x20]`을 구분해야 하며, `memcmp` 자체는 같을 때 0이지만 bool 래퍼는 성공 시 1을 반환한다.
- Next: Day087

### Day087
- Topic: Reversing — Ghidra 2: strings / XREF / caller 흐름
- Status: verification pending
- Result: 문자열 XREF에서 main과 성공·실패 출력 함수를 추적하고, `FUN_00101510`에는 코드 caller가 없음을 확인해 maintenance 문자열을 decoy로 판정했다. alias `route_87`, phrase `xref_guides_flow`를 복원하고 실제 성공을 확인했으며 GDB에서 `check_alias()` 직후 `EAX=1`을 확인했다.
- Files: Day040-100/Day087/README.txt, Day040-100/Day087/day87_challenge, Day040-100/Day087/wrtie_up.txt
- Problems: 필수 실패 경로 재현이 저장소에 없다. write-up 파일명도 요청된 `day87_rev.md`가 아닌 `wrtie_up.txt`이므로 다음 commit에서 정리한다.
- Next: Day088 evidence completion

### Day088
- Topic: Reversing — Ghidra 3: rename / type recovery
- Status: verification pending
- Result: 반복되는 offset으로 0x40바이트 `AnalysisContext` 구조를 복원했다. header 검사, handle 검사, phrase transform, digest 계산, 단계별 score 증가 흐름을 분석하고 정상 입력의 phrase `recover_the_type`와 성공 출력을 확인했다.
- Files: Day040-100/Day088/README.txt, Day040-100/Day088/day88_challenge
- Problems: commit에 `day88_rev.md`가 없고, custom structure를 실제 적용한 전후 화면/명령 기록, GDB 또는 objdump 교차검증, 실패 경로 재현이 없다.
- Next: Day087-Day088 verification completion

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
