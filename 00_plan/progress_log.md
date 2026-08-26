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

- Last completed: Day109
- Current focus: Day108에서는 Day107 정적 분석 결과를 IOC 노트로 재구성해 정확한 SHA-256, 조건부 파일 경로와 내용 표시자를 유효 IOC로 기록하고, caller가 없는 네트워크·셸 관련 decoy는 정상 실행 행위에서 제외했다. Confidence, false-positive risk, severity를 분리하고 정적 분석만으로 실제 파일 생성 성공과 환경 의존 동작을 확정할 수 없다는 한계를 명시했다. Day109에서는 stripped Day105 바이너리의 main과 inspect/submit/quit 흐름을 문서화했다. Ghidra와 objdump에서 `read(0, rbp-0x40, 0xa0)`을 교차검증해 56바이트 버퍼, canary `0x38`, saved RBP `0x40`, saved RIP `0x48`을 계산했다. Receipt는 앞부분 최대 24바이트의 32비트 계산값이라 canary leak이 아니며, incoming reference가 없는 zero-buffer `memcmp` 함수는 decoy로 판정했다. BOF는 존재하지만 canary 값, PIE base, libc base를 얻을 leak primitive가 없어 안정적인 exploit이 어렵다고 결론 냈다. 사용자 commits `1532db4`, `7ea7511`을 확인했다.
- Next task: Day110 — Rev portfolio checkpoint. Ghidra 분석 문서 8개를 점검하고 증거·추론·결론, 재현 명령, 파일 경로를 보완한 뒤 README와 `day110_rev.md`를 정리한다. 다음 공부 시작 전 `git pull`을 실행한다.
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


### Day100
- Topic: Reversing — Anti-debug 2: timing / trap
- Status: done
- Result: `clock_gettime(CLOCK_MONOTONIC)` 호출 사이의 루프 실행 시간을 계산하고 `cmp rax,0x2faf080; setg al`로 50ms 초과 여부를 반환하는 timing 검사를 복원했다. 루프에서 50ms 이상 정지해 `Timing anomaly detected`를 재현한 뒤 timing 함수 반환 직후 `EAX=0`으로 변경해 우회했다. trap 검사는 SIGTRAP handler 등록, 전역 flag 초기화, `INT3`, `test/sete` 순서이며 GDB가 SIGTRAP을 가로채 flag가 0으로 남으면 1을 반환해 `Trap anomaly detected`가 발생함을 확인했다. trap 함수 반환값도 0으로 변경해 정상 경로에 진입하고 `TIME_TRAP_100`으로 `ACCESS GRANTED`를 확인했다. objdump로 timing 상수와 `INT3` 이후 flag 검사 명령을 교차검증했다. CS에서는 monotonic clock의 필요성, SIGTRAP 전달 정책, Full RELRO, NX, ret2libc와 `pop rdi; ret`을 복습했다.
- Files: Day040-100/Day100/.gdb_history, Day040-100/Day100/SHA256SUMS, Day040-100/Day100/START_HERE.txt, Day040-100/Day100/day100_timing_trap_lab, Day040-100/Day100/write_up.txt
- Problems: `0x2faf080`은 80ms가 아니라 50,000,000ns인 50ms이며 `setg`이므로 정확히 50ms는 탐지하지 않는다. GDB가 의도적 `INT3`를 소비하면 프로그램 handler가 실행되지 않아 flag가 0으로 남는다. `handle SIGTRAP ... pass`의 전역 변경은 다른 trap/step 동작에도 영향을 줄 수 있으므로 특정 함수 반환값만 바꾸는 편이 영향 범위가 좁다. `set $eax=0` 우회는 현재 실행에만 유효하므로 재실행할 때 다시 적용해야 한다.
- Next: Day101


### Day101
- Topic: Reversing — Anti-debug 3: 우회 노트 / persistent branch patch
- Status: done
- Result: `ptrace(PTRACE_TRACEME)`의 원본 `RAX=-1`이 `cmp/sete/movzx`를 거쳐 wrapper 반환값 `EAX=1`이 되고, main의 `TEST → JNZ`가 탐지 실패 블록을 선택하는 흐름을 복원했다. `readelf -lW`로 실행 가능한 LOAD segment를 확인해 `file_offset = 0x1336 - 0x1000 + 0x1000 = 0x1336`을 계산했다. 원본을 보존한 채 복사본의 6바이트 `0f 85 8c 00 00 00`을 `90` 6개로 패치하고, GDB에서 `EAX=1`이 유지된 상태에서도 Token 입력 경로에 도달함을 확인했다. 패치본은 GDB 재실행 후에도 우회가 유지됐고 원본은 계속 탐지됐다. 원본 SHA-256은 `28e890295425379ce96d41e5dc14559e18acedc8f615c38b5d5dfbc7c8c6fefe`, 패치본은 `e78fcfee9f71de0991494be6db02cdf00f9edf56a06e6bde4012e76e5d437f03`이다. CS에서는 디스크 패치와 runtime patch의 지속성, NX·Full RELRO의 보호 대상, 무결성 검사와 GOT overwrite의 인자 호환성을 정리했다.
- Files: Day101-160/Day101/.gdb_history, Day101-160/Day101/SHA256SUMS, Day101-160/Day101/START_HERE.txt, Day101-160/Day101/day101_patch_lab, Day101-160/Day101/day101_patch_lab_patched, Day101-160/Day101/day101_rev.md, Day101-160/Day101/write_up.txt
- Problems: `-1`은 ptrace의 원본 반환값이고 main이 받는 wrapper 결과는 `1`이므로 두 관찰 지점을 구분해야 한다. Ghidra의 VA를 파일 오프셋으로 바로 가정하지 않고 해당 LOAD segment의 `p_vaddr/p_offset`으로 변환해야 한다. 6바이트 JNZ의 일부만 덮으면 남은 바이트가 잘못된 명령으로 해석될 수 있다. 정적 패치는 재실행 후에도 유지되지만 파일 해시와 코드가 바뀌므로 서명·self-integrity 검사에 탐지될 수 있다.
- Next: Day102


### Day102
- Topic: Reversing — Pwn from Ghidra 1: 취약점 위치 찾기
- Status: done
- Result: 안전한 `fgets` 경로와 `read(0, [rbp-0x30], 0x90)` 경로를 비교해 BOF를 찾았다. `[rbp-0x8]`의 TLS canary 저장·검사와 `__stack_chk_fail` 호출 조건을 확인했고, 출력 preview가 최대 16바이트라 후속 leak으로 바로 이어지지 않음을 구분했다.
- Files: Day101-160/Day102/SHA256SUMS, Day101-160/Day102/START_HERE.txt, Day101-160/Day102/day102_unsafe_call_lab, Day101-160/Day105/write_up.txt
- Problems: 입력 크기 제한과 출력 길이 제한은 서로 다른 보호다. Ghidra의 배열 표기만 믿지 않고 실제 `rbp` 상대 주소와 read 크기를 어셈블리에서 확인해야 한다.
- Next: Day103

### Day103
- Topic: Reversing — Pwn from Ghidra 2: stack offset 근거
- Status: done
- Result: 버퍼 시작점에서 canary, saved RBP, saved RIP까지의 stack layout을 어셈블리 피연산자로 계산했다. 실습 기준 offset은 canary `0x28`, saved RBP `0x30`, RIP `0x38`이었고, 160바이트 입력에서 RIP 뒤에 사용할 수 있는 공간은 80바이트였다.
- Files: Day101-160/Day105/write_up.txt
- Problems: Ghidra의 `local_XX` 이름은 실제 `[rbp-offset]`과 혼동될 수 있고, 디컴파일러가 padding까지 배열로 묶을 수 있다. 원본 소스의 선언 크기는 어셈블리만으로 확정하지 않는다.
- Next: Day104

### Day104
- Topic: Reversing — Pwn from Ghidra 3: exploitability 판단
- Status: done
- Result: NX, PIE, Canary, Full RELRO가 모두 활성화된 BOF를 대상으로 공격 전제조건을 정리했다. canary와 PIE를 유출할 primitive가 없으면 안정적인 제어 흐름 탈취가 어렵고, leak이 있다면 canary 보존 → PIE base 계산 → GOT read를 통한 libc leak → ret2libc 순서로 전환할 수 있음을 문서화했다.
- Files: Day101-160/Day105/write_up.txt
- Problems: BOF 존재와 exploit 가능성은 같은 결론이 아니다. Full RELRO는 GOT 쓰기를 막지만 GOT 읽기와 주소 leak 자체를 막지는 않는다.
- Next: Day105

### Day105
- Topic: Reversing — Stripped mini exam
- Status: done
- Result: `__libc_start_main`의 첫 번째 인자에서 main을 찾고 inspect/submit/quit 입력 흐름을 복원했다. submit의 56바이트 버퍼에 160바이트를 받는 BOF와 canary `0x38`, saved RBP `0x40`, RIP `0x48` offset을 확인했다. Receipt 출력은 입력 길이를 24로 줄여 처음 24바이트만 노출하므로 stack leak이 되지 않았다. 전체 문자열과 XREF를 조사해 incoming reference가 없는 zero-buffer `memcmp` 함수를 decoy로 판정했다.
- Files: Day101-160/Day105/SHA256SUMS, Day101-160/Day105/START_HERE.txt, Day101-160/Day105/day105_stripped_exam, Day101-160/Day105/write_up.txt
- Problems: read 반환값을 나중에 24로 고정해도 이미 발생한 overflow는 취소되지 않는다. 반대로 출력이 항상 버퍼 앞부분 24바이트라면 BOF만으로 canary/PIE 값이 노출되지는 않는다. decoy 여부는 문자열 존재가 아니라 실제 caller와 참조 가능성으로 판정해야 한다.
- Next: Day106


### Day106
- Topic: Reversing — Malware static 1: 안전 환경
- Status: done
- Result: 정적 분석 샘플의 식별 정보와 무결성 기록, VM 격리 설정, strings/imports/XREF 증거, 관찰·추론·미확정 주장, 재현 명령과 분석 한계를 하나의 보고서 양식으로 정리했다. 네트워크 Disconnected, 공유 기능 Disabled, 개인 계정 미사용, clean snapshot을 안전 기준으로 잡았고 샘플 실행 없이 분석하는 원칙을 확인했다. CS에서는 신뢰 경계, 최소 권한, 공격 표면, 다층 방어를 학습하고 snapshot은 VM 내부 상태만 복구하며 공유 폴더·정보 유출 같은 외부 피해는 취소하지 못함을 확인했다.
- Files: Day101-160/Day106/write_up.txt
- Problems: strings/imports의 존재와 실제 호출을 혼동하면 안 된다. capability를 판단하려면 XREF, caller 도달 가능성, 인자·반환값의 data flow가 필요하다. 격리는 위험을 완전히 제거하는 것이 아니라 VM과 호스트·내부망·외부망 사이 접점을 최소화하는 것이며, 스냅샷은 보안 경계 자체가 아니다. 이번 보고서는 가상 교육용 샘플을 전제로 작성했으므로 실제 실행 행위와 네트워크 결과는 검증하지 않았다.
- Next: Day107


### Day107
- Topic: Reversing — Malware static 2: strings/imports
- Status: done
- Result: 문자열과 undefined dynamic symbol을 1차 단서로 분류한 뒤 Ghidra XREF, incoming caller, 실제 인자와 반환값 data flow를 따라 capability를 판정했다. main에서 도달 가능한 기능은 XOR 복원 출력과 로컬 상태 파일 기록뿐이었다. 네트워크 함수는 미호출이고 invalid FD를 사용하며, `/bin/sh`는 단순 출력 문자열이고 미호출 execve 함수의 실제 대상은 `/usr/bin/true`임을 확인했다. objdump로 execve 인자 레지스터를 교차검증했다.
- Files: Day101-160/Day107/SHA256SUMS, Day101-160/Day107/START_HERE.txt, Day101-160/Day107/day107_static_lab, Day101-160/Day107/day107_rev.md, Day101-160/Day107/write_up.txt
- Problems: `strings`에 나타나는 import 이름은 `.dynstr`의 동적 심볼 문자열일 수 있으므로 독립된 두 증거로 중복 계산하면 안 된다. import 존재, 동적 로더의 주소 resolve, 도달 가능한 실제 call은 서로 다르다. 함수가 main에서 직접 호출되지 않더라도 callback·함수 포인터·constructor 등 간접 경로를 확인해야 하며, `write`의 capability는 FD가 `open`과 `socket` 중 어디서 왔는지에 따라 달라진다.
- Next: Day108


### Day108
- Topic: Reversing — Malware static 3: IOC note
- Status: done
- Result: Day107 샘플의 SHA-256을 exact-sample IOC로 기록하고, 정상 실행의 archive 분기에서 생성·덮어쓰기 되는 `/tmp/day107_status.log`와 `analysis_status=ready\n`을 조건부 host/content IOC로 분류했다. `198.51.100.20:4444` 네트워크 후보는 caller 부재와 invalid FD 사용 때문에, `/bin/sh`·`execve` 후보는 main에서 도달하지 않고 실제 execve 대상도 `/usr/bin/true`이므로 활성 행위에서 제외했다. Confidence, severity, false-positive risk를 서로 다른 판단 축으로 구분했다.
- Files: Day101-160/Day108/write_up.txt
- Problems: 해시는 동일 파일 식별에는 강하지만 한 바이트 변경에도 달라져 변종 탐지에 약하다. 경로·내용 IOC는 정상 프로그램과 겹칠 수 있으므로 단독 사용 시 오탐 가능성이 있다. 정적 분석만 수행해 실제 파일 생성·기록 성공과 간접 호출·환경 의존 분기를 모두 확인하지 못했다.
- Next: Day109

### Day109
- Topic: Reversing — Reversing write-up 2
- Status: done
- Result: stripped Day105 ELF에서 `__libc_start_main` 첫 번째 인자를 근거로 main을 찾고 inspect/submit/quit 흐름을 문서화했다. submit의 Ghidra listing과 objdump에서 `RDI=0`, `RSI=rbp-0x40`, `RDX=0xa0`을 교차검증해 56바이트 stack buffer에 160바이트를 쓰는 BOF와 canary `0x38`, saved RBP `0x40`, saved RIP `0x48` offset을 확인했다. Receipt 계산은 앞부분 최대 24바이트만 사용해 canary까지 도달하지 않으며, incoming reference가 없는 zero-buffer `memcmp` 함수는 decoy로 판정했다. 증거 → 추론 → 결론 구조로 write-up을 완성했다.
- Files: Day101-160/Day105/SHA256SUMS, Day101-160/Day105/START_HERE.txt, Day101-160/Day105/day105_stripped_exam, Day101-160/Day109/write_up.txt
- Problems: `fgets(..., 0x20, ...)`은 최대 31문자와 NUL을 저장한다. read 반환값을 나중에 `0x18`로 제한해도 이미 발생한 overflow는 취소되지 않는다. BOF로 saved RIP까지 덮을 수 있어도 canary 값, PIE base, libc base를 얻는 leak primitive가 없으면 안정적인 exploit으로 이어지지 않는다.
- Next: Day110

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
