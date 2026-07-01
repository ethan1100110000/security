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
- 사용자에게 먼저 아래 명령을 실행하게 한다.

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
- 새 보안 프로젝트 채팅에서도 pwn/reversing 문제를 풀 때는 단순히 BOF/FSB 등 명확한 취약점을 exploit하는 데서 끝내지 않는다.
- 매 문제마다 다음 루틴을 기본으로 적용한다: 입력 지점 확인 → 입력값 data flow 추적 → 검증/제한 위치 확인 → 위험 sink 확인 → 취약점 root cause 설명 → 소스 없이 찾는 방법 설명 → exploitability 판단 → exploit 작성/검증.
- 사용자가 별도로 생략하라고 하지 않는 한, write-up 또는 복기 단계에 root cause와 discovery 관점을 포함한다.

---

## Current Pointer

- Last completed: Day058
- Current focus: Canary/PIE/libc/pivot 통합 체크 + shellcode 전략 정리, Day57-58 CS Fundamentals 완료
- Next task: Day059 미니시험 2 진행
- Repo rule: 각 Day 폴더 안에 그날의 바이너리, 소스, exploit, write-up, 실행 결과를 넣는다.

---

## Daily Log

### Day040
- Topic: Canary + PIE + BOF
- Status: done
- Result: canary/PIE leak 후 PIE-adjusted win 진입 성공
- Next: Day041 FSB leak

### Day041
- Topic: FSB leak lab
- Status: done
- Result: canary leak, PIE leak, PIE base 계산 검증
- Next: Day042

### Day042
- Topic: Canary + BOF ret2win
- Status: done
- Result: canary 보존 + RIP 제어 성공
- Next: Day043

### Day043
- Topic: PIE ON ret2win
- Status: done
- Result: PIE leak, PIE base 계산, win 진입 성공
- Next: Day044

### Day044
- Topic: Canary + PIE + BOF
- Status: done
- Result: canary/PIE leak + BOF 성공
- Next: Day045

### Day045
- Topic: Failure review + dynamic linking
- Status: done
- Result: canary/PIE/parsing/same-process leak 체크리스트 정리
- Next: Day046

### Day046
- Topic: External PIE OFF ret2libc
- Status: done
- Result: ld/libc 매칭 문제 해결, ret2libc 성공
- Next: Day047

### Day047
- Topic: External PIE ON ret2libc
- Status: done
- Result: PIE base 계산 + libc leak + system('/bin/sh') 성공
- Next: Day048

### Day048
- Topic: Shellcode NX OFF
- Status: done
- Result: stack buffer leak, offset 136, execve('/bin/sh') shellcode 성공
- Next: Day049

### Day049
- Topic: CS backlog / shellcode review
- Status: done
- Result: syscall vs libc, NX/execstack, execve register 정리
- Next: Day050

### Day050
- Topic: FSB / pwn review
- Status: done
- Result: Day051 전 단계 복기
- Next: Day051

### Day051
- Topic: FSB byte write
- Status: done
- Result: printf(buf) 기반 FSB offset 확인, %hhn / fmtstr_payload로 target byte overwrite 성공
- Next: Day052

### Day052
- Topic: FSB leak + write / GOT overwrite
- Status: done
- Result: stripped PIE 바이너리에서 FSB로 PIE leak 후 base 계산, win 후보(0x1189)와 main offset(0x1226) 식별, exit@got offset(0x4028)을 runtime 주소로 보정해 `exit@got -> win` overwrite 성공
- Files: Day040-100/Day052
- Problems: PIE ON에서 GOT offset도 `pie_base + offset`으로 보정해야 함을 재확인. stripped 상태에서는 심볼명 대신 `_start`, `__libc_start_main`, `system@plt`, `strings -tx`, `readelf -rW`, `objdump -d -M intel`로 함수 역할을 판단해야 함.
- Next: Day053

### Day053
- Topic: Stack pivot 기본형 + hard staged read
- Status: done
- Result: 기본형에서 `.bss` 전역 `fake_stack`에 fake chain `[dummy rbp][ret][win]`을 구성하고, stage2 BOF로 `saved rbp = fake_addr`, `saved rip = leave; ret`를 넣어 pivot 흐름을 검증했다. hard형에서는 전역 fake stack 입력 없이 1차 ROP로 `read(0, fake_addr, size)`를 호출해 writable 영역에 fake stack을 작성한 뒤, ROP chain 마지막의 `leave; ret`로 pivot해 win 실행에 성공했다.
- Files: Day040-100/Day053
- Problems: `rbp`와 `[rbp]`, `fake_addr`와 `[fake_addr]`를 구분하는 것이 핵심이었다. 첫 번째 `leave`는 `rbp = fake_addr`를 만들고, 두 번째 `leave`는 `rsp = fake_addr`, `rbp = [fake_addr]`를 만든다. `.bss`/RW 영역 시작점은 stdout/stdin/stderr나 런타임 데이터가 있을 수 있고, fake stack 시작점이 너무 낮으면 함수 내부 `push/call`로 `rsp`가 낮은 주소로 내려가 터질 수 있어 fake stack을 writable 영역 안쪽에 배치해야 한다. `read` 입력에는 `sendline()`보다 `send()`가 안정적이며, `sendline()`의 남은 `\n`이 다음 `read`에 섞일 수 있음을 확인했다.
- Next: Day054 진행 전 Day53 복습 + CS Fundamentals 묶음(virtual memory, page alignment, PIE base)

### Day054
- Topic: Pivot ret2libc + Day53-54 CS Fundamentals
- Status: done
- Result: 작은 BOF에서 `leave; ret`로 `.bss` fake stack에 pivot한 뒤, 1차 chain으로 `puts(puts_got)` leak, libc base 계산, main 복귀, 2차 chain으로 `system("/bin/sh")` 호출에 성공했다. Day53 CS(page alignment / PIE base)와 Day54 CS(ELF segment / vmmap 권한)도 함께 완료했다.
- Files: Day040-100/Day054
- Problems: main 복귀 후 2차 입력은 `round_no` 분기 때문에 `fake_stack2`로 들어가는데 처음에는 다시 `fake_stack1`로 pivot해서 leak chain 루프가 반복됐다. `disas vuln`에서 `round_no` 분기와 `read` 직전 `rsi`를 확인해 입력 목적지와 pivot 주소를 맞춰 해결했다. fake stack은 실행 코드가 아니라 ROP 주소 목록이므로 `rw-p`면 충분하고, 실제 실행은 `.text/.plt/libc`의 `r-xp` 영역에서 일어난다는 점을 정리했다.
- Next: Day055

### Day055
- Topic: FSB/Pivot write-up 품질 보강 + saved RIP overwrite 복습
- Status: done
- Result: Day52 FSB GOT overwrite와 Day54 pivot ret2libc의 write-up 보강 포인트를 정리했다. Day52에서는 exit@got 선택 이유, PIE ON에서 GOT/function offset 보정, %hn으로 2바이트씩 나눠 쓰는 이유를 복습했다. Day54에서는 NX ON에서도 ROP pivot이 가능한 이유, 소스 없이 read 인자의 rsi와 disas vuln으로 fake_stack1/fake_stack2를 확인하는 방법, 2차 chain 저장 위치와 pivot 대상 주소 불일치 실패 원인을 정리했다. saved rbp/saved rip의 역할과 leave; ret pivot 흐름도 복습했다.
- Files: Day040-100/Day055/review_notes.txt
- Problems: 2차 입력이 fake_stack2에 저장되는데 pivot을 fake_stack1로 반복하면 이전 puts leak chain이 다시 실행되어 main 복귀 루프가 생긴다는 점을 재확인했다. saved rbp는 fake stack 주소로, saved rip는 leave; ret gadget으로 덮어야 pivot이 성립한다.
- Next: Day056

### Day056
- Topic: ROP Emporium ret2csu + custom ret2csu hard + calling convention 복습
- Status: done
- Result: ROP Emporium `ret2csu` 개념을 복구하고, `__libc_csu_init`의 pop gadget과 call gadget을 이용해 레지스터 인자 전달 흐름을 정리했다. 이어 custom stripped/no PIE/NX/Full RELRO ret2csu hard 문제를 풀었다. 1차 csu chain으로 `read(0, bss, 8)`을 호출해 writable `.bss`에 `win` 주소를 저장하고, 2차 csu chain에서 `r12=bss`, `rbx=0`으로 `call [bss]`를 수행해 `win(arg1,arg2,arg3)` 호출에 성공했다. CS로 x86-64 calling convention을 복습하며 `rdi/rsi/rdx` 인자 전달, `read`, `write`, `system`, `execve`의 레지스터 세팅 차이를 정리했다.
- Files: Day040-100/Day056
- Problems: `call [r12+rbx*8]`는 `r12` 자체를 호출하는 것이 아니라 `r12+rbx*8` 위치의 8바이트 함수 포인터를 읽어 호출한다. 따라서 `r12=win`이 아니라 `[bss]=win`, `r12=bss` 구조가 필요했다. `rbx=0`, `rbp=1`로 csu loop를 1회만 돌리고, `csu_call` 이후 `add rsp,8`과 6개의 pop을 처리하기 위해 cleanup dummy 7개가 필요함을 확인했다. 기본 ret2csu에서는 `mov edi,r13d` 때문에 64비트 첫 번째 인자를 온전히 넣기 어렵고, hard 변형에서는 staged read 후 indirect call 방식으로 해결했다. staged read에는 `sendline(p64(win))`보다 `send(p64(win))`이 적절함도 확인했다. Mac Docker에서는 Apple Silicon 기본 ARM64 컨테이너로 x86-64 ELF 실행이 안 되어 `--platform linux/amd64`가 필요했으며, 이후 WSL에서 동일한 방식으로 성공했다.
- Next: Day057 ROP 문제 B + 실패 케이스 문서화, 가능하면 PIE ON + no FSB 우회/partial overwrite 입문 보강

### Day057
- Topic: ret2csu + xchg pivot 복합 문제 2개 풀이
- Status: done
- Result: custom Problem 1에서 No PIE/No Canary/NX/Full RELRO 환경의 heap fake stack + `xchg rax,rsp; ret` pivot + ret2csu staged indirect call을 성공했다. stack payload는 `pop rax; ret`, heap leak 주소, `xchg rax,rsp; ret`로 pivot만 수행하고, heap payload가 실제 ret2csu chain을 담당했다. 1차 csu로 `read(0, slot, 8)`을 호출해 `.bss` slot에 `win` 주소를 저장하고, 2차 csu에서 `r12=slot`, `call [slot]`로 `win(arg1,arg2,arg3)` 호출에 성공했다. custom Problem 2에서는 PIE ON/Canary ON 환경에서 FSB로 canary와 PIE code pointer를 leak하고, PIE base 계산 후 gadget/GOT/.bss/win 주소를 보정하고 canary를 보존한 뒤 같은 xchg pivot + ret2csu 구조로 성공했다.
- Files: Day040-100/Day057
- Problems: `xchg rax,rsp; ret` pivot에서는 fake stack 첫 qword가 dummy rbp가 아니라 첫 RIP가 되어야 한다. 반대로 `leave; ret` pivot은 fake stack 첫 qword가 dummy rbp이고 두 번째 qword가 첫 RIP다. ret2csu의 `call [r12+rbx*8]` 때문에 `r12=read@plt` 또는 `r12=win`처럼 함수 주소를 직접 넣으면 실패하고, `r12=read@got` 또는 `r12=slot`처럼 함수 포인터가 저장된 메모리 주소를 넣어야 한다. Full RELRO에서는 GOT overwrite는 불가능하지만 GOT read/call은 가능하며, 쓰기 대상은 writable `.bss` slot이어야 한다. CS Fundamentals는 시간 부족으로 Day58에 통합 체크와 함께 이월한다.
- Next: Day058 Canary/PIE/libc/pivot 통합 체크 + shellcode, pivot 방식 이론 비교 추가

### Day058
- Topic: Canary/PIE/libc/pivot 통합 체크 + shellcode + Day57-58 CS Fundamentals
- Status: done
- Result: `pwn_integrated_checklist.md`에 Canary/PIE/NX/RELRO 기준 exploit 전략 분기, ret2libc leak 흐름, puts leak과 write leak 차이, NX OFF shellcode 판단, execve shellcode 레지스터, pivot 방식 비교를 정리했다. `leave; ret`, `xchg reg,rsp; ret`, `pop rsp; ret`, `mov rsp,reg; ret`, `add rsp,imm; ret`의 fake stack 구조 차이를 비교했고, FSB leak + BOF + PIE/Canary + libc leak + main 복귀 + 2차 system 구조를 통합 체크했다. 추가 고난도 Q/A로 raw bytes leak 파싱, libc base 계산, system 내부 movaps alignment crash, PIE ON ret2csu 주소 보정, staged read 입력 타이밍을 점검했다. Day57 CS인 보호기법별 우회 전략 분류와 Day58 CS인 syscall vs libc 함수 차이도 함께 완료했다.
- Files: Day040-100/Day058/pwn_integrated_checklist.md
- Problems: raw leak은 ASCII hex가 아니므로 `int(..., 16)`이 아니라 `u64(leak.ljust(8, b"\x00"))`로 복구해야 한다. PIE ON에서는 `.text`, `.plt`, `.got`, `.bss`, csu gadget, win offset에 PIE base를 더해야 하지만 canary, heap leak, libc leak, 단순 인자에는 더하면 안 된다. Full RELRO는 GOT overwrite를 막지만 GOT read/leak/call은 가능하다. `system` 내부 `movaps`에서 터지면 `/bin/sh` 주소보다 stack alignment를 먼저 의심하고, 필요하면 `ret` gadget으로 `rsp` 정렬을 바꾼다. `execve` syscall은 libc `system`과 달리 `rax=59`, `rdi/rsi/rdx` 세팅과 `syscall` 명령이 핵심이다.
- Next: Day059 미니시험 2

---

## Update Template

### DayXXX
- Topic:
- Status:
- Result:
- Files:
- Problems:
- Next:
