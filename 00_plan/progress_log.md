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

- Last completed: Day099
- Current focus: Day099에서는 stripped PIE 바이너리에서 `ptrace(PTRACE_TRACEME, 0, 0, 0)` 기반 anti-debug 함수를 찾고, 원본 반환값 `-1`을 bool `true(1)`로 변환해 `Debugger detected` 후 종료하는 흐름을 복원했다. GDB에서 PIE base `+0x124b`의 ptrace 호출 직후 `RAX=-1`을 확인하고 `set $rax=0`으로 바꿔 탐지 분기를 우회해 Token 입력 지점까지 진행했다. 검증 함수의 `strlen(input)==12`와 `diff |= input[i] ^ data1[i] ^ data2[i]`를 분석해 `input[i]=data1[i]^data2[i]`로 역산하고 `TRACE_BYPASS`로 성공했다. 같은 길이 오답과 13바이트 입력 실패도 확인했다. CS에서는 일시적 레지스터 변경과 정적 패치, LD_PRELOAD 심볼 interposition, PLT/GOT 연결 및 x86-64 `execve` shellcode 레지스터를 정리했으며 사용자 commit `70d553f`을 확인했다.
- Next task: Day100. 새 채팅에서 먼저 `git pull`을 실행한 뒤 이 파일과 최신 `보안 계획표.xlsx`의 Day100 행을 확인한다.
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
- Status: study done; repository cleanup pending
- Result: 문자열 XREF에서 main과 성공·실패 출력 함수를 추적하고, `FUN_00101510`에는 코드 caller가 없음을 확인해 maintenance 문자열을 decoy로 판정했다. alias `route_87`, phrase `xref_guides_flow`를 복원하고 실제 성공을 확인했으며 GDB에서 `check_alias()` 직후 `EAX=1`을 확인했다.
- Files: Day040-100/Day087/README.txt, Day040-100/Day087/day87_challenge, Day040-100/Day087/wrtie_up.txt
- Problems: 파일명 `wrtie_up.txt` 오타와 실패 경로 기록은 이후 저장소 정리 시 보완한다.
- Next: Day088

### Day088
- Topic: Reversing — Ghidra 3: rename / type recovery
- Status: study done; repository cleanup pending
- Result: 반복되는 offset으로 0x40바이트 `AnalysisContext` 구조를 복원했다. header 검사, handle 검사, phrase transform, digest 계산, 단계별 score 증가 흐름을 분석하고 정상 입력의 phrase `recover_the_type`와 성공 출력을 확인했다.
- Files: Day040-100/Day088/README.txt, Day040-100/Day088/day88_challenge
- Problems: standalone write-up과 custom type 적용 전후 증거는 이후 저장소 정리 시 보완한다.
- Next: Day089

### Day089
- Topic: Reversing — Stripped 1: entry에서 main 찾기
- Status: done
- Result: ELF entry에서 `__libc_start_main` 호출을 추적해 `FUN_0010148c`를 main으로 복원했다. GDB에서 호출 시점의 `RDI`를 확인하고 `RDI - 0x148c`가 PIE base와 일치함을 검증했다. checker의 XOR 조건을 역산해 `undry_umai_aaa`로 성공했으며 마지막 세 문자는 길이만 검사되고 실제 비교에서 누락된 under-validation임을 확인했다.
- Files: Day040-100/Day089/.gdb_history, Day040-100/Day089/README.txt, Day040-100/Day089/day89_challenge
- Problems: ELF entry는 main이 아니라 `_start`이며, 문자열 XREF만으로 main을 확정하지 않고 `__libc_start_main`의 첫 번째 인자 흐름을 근거로 삼아야 한다.
- Next: Day090

### Day090
- Topic: Reversing — Stripped 2: 함수 경계 복원
- Status: done
- Result: 정상 입력 `boundary_xref_90`과 출력 tag `875a32e8`을 확인했다. `FUN_001011c0`은 `0x0010142d`의 직접 call XREF, `EDI/SIL` 인자 사용, `EAX` 반환값, `0x001011cc`의 `ret`을 근거로 독립 leaf 함수로 판정했다. 경계는 `0x001011c0–0x001011cc`, 크기는 13바이트이며 표준 frame prologue가 없어도 함수일 수 있음을 확인했다.
- Files: Day040-100/Day090/README.txt, Day040-100/Day090/day90_challenge, Day040-100/Day090/write_up.txt
- Problems: `push rbp; mov rbp,rsp`는 함수 판정의 필수 조건이 아니다. `ret` 뒤의 NOP/UD2는 함수 본문이 아닌 패딩 또는 트랩 영역으로 구분해야 한다.
- Next: Day091

### Day091
- Topic: Reversing — Stripped 3: control-flow 문서화
- Status: done
- Result: ELF entry에서 `__libc_start_main`의 첫 번째 인자 `RDI`를 추적해 main offset `0x161a`를 복원했다. main은 stdin/stdout buffering을 설정한 뒤 `0x1589`를 호출하며, 처리 함수는 `0x11d0`에서 입력을 받고 실패를 분기한 뒤 `0x1458`의 검증 결과를 `0x14d0`에 전달해 성공·실패 출력을 선택한다. caller/callee, 인자 레지스터, 반환값 검사와 분기 방향을 기준으로 전체 흐름을 정리했다.
- Problems: 서로 다른 바이너리의 주소를 섞으면 main 후보와 함수 경계를 잘못 판정할 수 있다. 항상 현재 ELF의 entry와 `lea rdi, [rip+...]` 계산을 먼저 고정한 뒤 call graph를 작성해야 한다.
- Next: Day092

### Day092
- Topic: Reversing — Crackme 1: 분기 조건 분석
- Status: done
- Result: stripped PIE의 `_start`에서 `lea rdi, [rip+0x2af]`를 추적해 main offset `0x135e`를 확인했다. 입력 래퍼의 `fgets`/`strcspn` 흐름과 검증 함수의 `strlen(input) == 8`을 복원한 뒤, equality·범위·문자 간 합/XOR·비트 마스크 조건을 조합해 성공 입력 `R5AaYBGN`을 만들었다. 실제 실행에서 `ACCESS GRANTED`를 확인하고 한 조건을 바꾼 입력으로 `ACCESS DENIED`도 확인했다.
- Files: Day040-100/Day092/day92_crackme, Day040-100/Day092/s, Day040-100/Day092/write_up.txt
- Problems: `cmp src/dst` 피연산자 순서와 `jg`의 signed 해석, `jb`의 unsigned 해석을 분리해야 한다. `movzx/movsx` 확장 방식과 `OF/CF`의 의미는 추가 복습이 필요하다.
- Next: Day093

### Day093
- Topic: Reversing — Crackme 2: loop verifier 역산
- Status: done
- Result: 10바이트 입력을 순회하는 검증 루프를 분석해 `key_index = (3*i+1)%10`, key XOR, `ROL8((i%3)+1)`, `+7*i+3`의 정방향 식을 복원했다. 비교 대상에서 `7*i+3`을 빼고, `ROL8`을 `ROR8`로 되돌린 뒤 같은 key 바이트로 XOR해 성공 입력 `R3-L00P-7N`을 구했으며 실제 실행에서 `ACCESS GRANTED`를 확인했다. CS에서는 `strcmp`와 `memcmp`의 종료 조건 차이, 부분 비교, 널 종료 누락 및 잘못된 길이로 인한 범위 밖 읽기와 UB를 정리했다.
- Files: Day040-100/Day093/.gdb_history, Day040-100/Day093/day93_crackme, Day040-100/Day093/write_up.txt
- Problems: Ghidra의 `'\a'`와 `'\x03'`는 각각 7과 3이며, `target[i]`에서 이 덧셈을 먼저 제거해야 함수 반환값을 얻을 수 있다. 회전 함수의 역연산 결과는 입력 자체가 아니라 XOR 직후 값이므로 같은 key로 한 번 더 XOR해야 한다. 모든 중간값은 1바이트로 맞춘다.
- Next: Day094


### Day094
- Topic: Reversing — Crackme 3: keygen logic / chained state verifier
- Status: done
- Result: 입력에서 시작해 각 반복의 상태 변화가 다음 반복으로 전달되고 최종 결과 배열과 비교되는 chained-state verifier 흐름을 분석했다. 독립적인 바이트 조건이 아니라 이전 state에 의존하는 순차 계산이라는 점을 기준으로 keygen 로직의 loop/state 추적을 완료했다.
- Files: Day040-100/Day094/day94_crackme
- Problems: 이전 반복의 state를 갱신하기 전에 덮어쓰면 데이터 의존성을 잃는다. 각 연산의 8비트 절삭과 정방향 계산 순서를 유지해야 한다.
- Next: Day095

### Day095
- Topic: Reversing — 분석 write-up과 C-like pseudo-code
- Status: done
- Result: 12바이트 입력의 개행 제거와 길이 검사, `state = 0x3d`에서 시작하는 변환 루프, `memcmp(calculated, target, 12)` 성공 조건을 문서화했다. `key[(5*i+2)%12] ^ input[i]`, 이전 state와 `3*i`의 덧셈, `ROL8`, `(0xa5-7*i) ^ new_state`를 의사코드로 정리하고 역산 순서를 함께 기록했다. CS에서는 의사코드에 분기·반복, 상태 의존성, 자료형 크기와 8비트 오버플로를 보존해야 함을 정리했다.
- Files: Day040-100/Day095/write_up.txt
- Problems: `old_state → new_state` 의존성을 유지해야 하며, `movzx`와 `movsx`의 0 확장·부호 확장을 구분해야 한다. write-up에는 objdump/GDB 교차검증 계획만 기록되어 있다.
- Next: Day096

### Day096
- Topic: Reversing — CFG 1: branch/loop 복원
- Status: done; GDB/objdump cross-check skipped by user
- Result: 어셈블리를 6개 basic block으로 나누고 `B5 → B2`의 back edge와 `B2 → B6`의 종료 간선을 찾아 반복문을 복원했다. `ecx < 4`인 동안 반복하고 짝수일 때만 `eax += ecx`를 수행하는 C-like 의사코드를 작성했으며 최종 `eax = 2`를 계산했다. CS에서는 조건부 점프는 보통 2개, 무조건 점프는 1개, `ret`은 함수 내부에서 0개의 후속 경로를 가지며 일반적인 함수 내부 CFG에서는 `call` 뒤 실행이 이어질 수 있음을 정리했다.
- Files: Day040-100/Day096/write_up.txt
- Problems: `test ecx, 1; jne`에서 홀수는 `add`를 건너뛰고 짝수는 fall-through한다. basic block의 총개수와 한 블록의 outgoing edge 수를 구분해야 한다. 계획표의 GDB/objdump 교차검증은 사용자 요청으로 생략했다.
- Next: Day097

### Day097
- Topic: Reversing — CFG 2: switch/jump table
- Status: done
- Result: stripped PIE의 switch 함수에서 `a-0x14`로 case 인덱스를 만들고 `ja`로 0~7 범위를 검사한 뒤, 4바이트 signed 상대 오프셋을 `movsxd`로 확장해 테이블 기준 주소에 더하고 간접 점프하는 흐름을 복원했다. 두 번째 함수의 `ROR32` 조건을 역산해 `b=0x4d2a9c17`을 구하고 case별 결과를 비교해 `a=0x1a`를 찾았으며 최종 입력 `26 1294638103`으로 성공했다. GDB에서 인자, case 인덱스, 초기 XOR 값과 실제 jump table 경로를 확인했다. CS에서는 case 밀도에 따른 `cmp`/분기 트리/jump table 선택, `ja`의 `CF=0 && ZF=0` 조건, signed 상대 오프셋의 부호 확장을 정리했다.
- Files: Day040-100/Day097/.gdb_history, Day040-100/Day097/day97_switch_lab, Day040-100/Day097/write_up.txt
- Problems: Ghidra의 함수 이름은 stripped 바이너리의 심볼이 아니며 PIE에서는 파일상 offset `0x1260`에 runtime base를 더해 breakpoint를 걸어야 한다. jump table 항목은 절대주소가 아니라 테이블 기준의 signed 32비트 상대 오프셋이므로 `mov ecx`의 zero-extension과 `movsxd rcx`의 sign-extension을 구분해야 한다.
- Next: Day098


### Day098
- Topic: Reversing — CFG 3: indirect call / function pointer
- Status: done
- Result: `param_1-0x14`의 unsigned 범위 검사와 함수 포인터 테이블을 추적해 입력 20~23이 각각 0x1300, 0x13d0, 0x14b0, 0x15a0에 매핑됨을 복원했다. `call qword ptr [rcx+rax*8]`에서 RCX는 테이블 시작 주소, RAX는 인덱스이며 `RDI=입력`, `RSI=출력 버퍼`, `RDX=12`로 전달됨을 확인했다. 0x14b0 함수의 ROL8, XOR, 연쇄 state 갱신을 역산해 성공 입력 `22 CALL_PTR_98!`을 구하고 실제 성공을 확인했다. GDB에서 `EAX=2`, `[RCX+RAX*8]=PIE base+0x14b0`, 실제 함수 진입과 인자를 검증했으며 `22 CALL_PTR_98?`와 `21 CALL_PTR_98!`은 모두 실패했다. CS에서는 인덱스 범위 검사, 함수 포인터 테이블 변조, 읽기 전용 배치와 CFI, NX 환경에서의 기존 실행 코드 재사용을 정리했다.
- Files: Day040-100/Day098/.gdb_history, Day040-100/Day098/SHA256SUMS, Day040-100/Day098/START_HERE.txt, Day040-100/Day098/day98_indirect_lab, Day040-100/Day098/day98_rev.md, Day040-100/Day098/write_up.txt
- Problems: 간접 호출 분석의 핵심은 모든 후보 함수의 복잡한 연산을 푸는 것이 아니라 입력에서 인덱스와 함수 포인터가 만들어지는 경로를 먼저 복원하고, 가능한 callee 중 성공 조건을 만족하는 경로를 선별하는 것이다. 인덱스가 정상 범위여도 테이블 항목 자체가 변조되면 호출 대상이 바뀌므로 범위 검사만으로는 충분하지 않다.
- Next: Day099


### Day099
- Topic: Reversing — Anti-debug 1: ptrace pattern
- Status: done
- Result: `ptrace(PTRACE_TRACEME)`의 성공·실패 반환값을 bool로 변환하는 anti-debug wrapper와 caller의 종료 분기를 복원했다. GDB 추적 중 ptrace 호출 직후 `RAX=-1`을 확인한 뒤 `RAX=0`으로 변경해 wrapper 반환값을 false로 만들고 정상 Token 검증 흐름에 진입했다. 검증 함수는 12바이트 입력에 대해 `diff |= input[i] ^ data1[i] ^ data2[i]`를 누적하고 `diff==0`일 때 성공하므로 `input[i]=data1[i]^data2[i]`로 역산해 `TRACE_BYPASS`를 구했다. 실제 `ACCESS GRANTED`와 `TRACE_BYPAS?`, `TRACE_BYPASSX` 실패를 확인했다. CS에서는 동적 레지스터 우회, 정적 명령어 패치, LD_PRELOAD를 통한 동적 심볼 선점과 PLT/GOT 연결을 비교했고, shellcode 복습에서 `execve("/bin/sh", NULL, NULL)`의 `RAX=59`, `RDI=문자열 주소`, `RSI=0`, `RDX=0`을 정리했다.
- Files: Day040-100/Day099/.gdb_history, Day040-100/Day099/SHA256SUMS, Day040-100/Day099/START_HERE.txt, Day040-100/Day099/day99_ptrace_lab, Day040-100/Day099/day99_rev.md, Day040-100/Day099/write_up.txt
- Problems: `PTRACE_TRACEME`는 GDB를 직접 식별하는 것이 아니라 이미 tracer가 존재해 요청이 실패하는지를 이용한다. sandbox나 보안 정책이 ptrace를 막아도 `-1`이 반환될 수 있어 오탐이 가능하다. `set $rax=0`은 현재 프로세스에서만 유효하며 재실행 시 다시 우회해야 하고, 지속적인 우회는 정적 패치나 함수 interposition 같은 별도 방법이 필요하다.
- Next: Day100

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
