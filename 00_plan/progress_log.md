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

- Last completed: Day053
- Current focus: Stack pivot 기본형/심화형 완료, leave; ret 기반 pivot 흐름 정리
- Next task: Day054 진행 전 Day53 stack pivot 복습 + CS Fundamentals 묶음 진행: virtual memory, page alignment, PIE base
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

---

## Update Template

### DayXXX
- Topic:
- Status:
- Result:
- Files:
- Problems:
- Next: