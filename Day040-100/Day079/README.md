# Day079 — Unsorted-bin leak → ret2libc lab

완성 exploit은 포함되어 있지 않다.

## 실행

```bash
unzip day079_lab_only.zip
cd day079_lab_only
chmod +x run.sh
./run.sh
make check
```

## 보호기법

- Canary: OFF
- NX: ON
- PIE: OFF
- ASLR: 시스템 설정에 따라 ON

## 프로그램 흐름

```text
malloc(0x500) victim
→ malloc(0x20) guard
→ free(victim)
→ freed user area의 앞 8바이트 출력
→ vuln()에서 두 번째 입력
```

## 네가 직접 해야 하는 것

1. 출력된 8바이트를 정확히 수신하고 `u64`로 파싱한다.
2. 해당 포인터가 libc의 어느 mapping에 속하는지 확인한다.
3. 현재 glibc의 leak offset을 GDB 또는 `/proc/<pid>/maps`로 구한다.
4. `libc_base = leak - leak_offset`을 계산한다.
5. 계산한 base를 `vmmap`의 offset-0 libc mapping과 비교한다.
6. `vuln()`의 saved RIP offset을 직접 구한다.
7. 바이너리의 고정 gadget과 libc의 `system`, `"/bin/sh"` 주소를 구한다.
8. stack alignment를 검증한다.
9. ROP payload를 직접 작성한다.
10. 틀린 leak offset을 넣었을 때의 실패도 확인한다.

## 권장 검증

```bash
gdb -q ./day79
```

확인 항목:

```text
- victim이 guard 때문에 top chunk와 병합되지 않는가
- free 후 victim[0:8]에 libc pointer가 들어가는가
- libc base가 page-aligned인가
- 계산한 system 주소가 libc r-xp에 속하는가
- "/bin/sh" 주소에서 문자열이 실제로 보이는가
- saved RIP offset이 cyclic 결과와 disassembly에서 일치하는가
- system 진입 직전 stack alignment가 맞는가
```
