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

---

## Current Pointer

- Last completed: Day051
- Current focus: FSB write / format string exploitation 복기 및 확장
- Next task: Day052 진행
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

---

## Update Template

### DayXXX
- Topic:
- Status:
- Result:
- Files:
- Problems:
- Next:
