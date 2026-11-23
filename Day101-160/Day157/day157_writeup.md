# Day157 — PLT/GOT, 동적 링커, libc 주소 교차검증 (진행 기록)

- 학습일: 2026-11-23 / FINAL W07 1/5
- 실습 대상: 교사 제공 `day157_relay.elf` (교육용 stripped PIE ELF)
- 상태: **메인 개념·CS·이전 ROP 회상 및 지연 D+2 복습 수행; 원본 실행 로그/환경과 최종 Git 증빙 미제출로 Day 전체 완료 보류.**
- 기록 근거: 사용자가 채팅에 복사한 `readelf`, `objdump`, `pwndbg` 출력과 일부 실행 결과의 구두 보고. 전체 터미널 로그를 보존한 것은 아님.

## 1. 정적 분석 — 보호기법과 relocation

사용자 확인: Canary, NX, PIE 활성화, Partial RELRO. Partial RELRO라는 사실만으로 GOT 전체 쓰기 가능을 확정하지 않고 동적 매핑과 구분함.

사용자 `readelf -r ./day157_relay.elf` 원문에서 발췌:

```text
.rela.dyn
0x3fc0  R_X86_64_GLOB_DAT   __libc_start_main@GLIBC_2.34
.rela.plt
0x4000  R_X86_64_JUMP_SLOT  strncmp@GLIBC_2.2.5
0x4008  R_X86_64_JUMP_SLOT  puts@GLIBC_2.2.5
0x4010  R_X86_64_JUMP_SLOT  write@GLIBC_2.2.5
0x4018  R_X86_64_JUMP_SLOT  strlen@GLIBC_2.2.5
0x4028  R_X86_64_JUMP_SLOT  printf@GLIBC_2.2.5
```

학생은 PLT가 GOT의 간접 점프 슬롯을 참조하며, 슬롯 내용과 슬롯 자체의 주소가 다름을 설명. 교사의 안내 이후 `puts@plt` 진입점에서 GOT 내용을 직접 관찰함.

## 2. 동적 분석 — puts 첫 호출 전후

사용자 제출한 Pwndbg 출력:

```text
pwndbg> x/3i $pc
0x555555555040 <puts@plt>: jmp QWORD PTR [rip+0x2fc2] # 0x555555558008 <puts@got.plt>
0x555555555046 <puts@plt+6>: push 0x1
0x55555555504b <puts@plt+11>: jmp 0x555555555020

pwndbg> x/gx 0x555555558008
0x555555558008 <puts@got.plt>: 0x0000555555555046
pwndbg> x/i *(void **)0x555555558008
0x555555555046 <puts@plt+6>: push 0x1
```

따라서 이 관찰 시점의 GOT 슬롯은 libc `puts`가 아닌 PLT 내부 해석 경로를 가리킴. 학생이 첫 호출 종료 후 보고한 GOT 슬롯 값은 `0x00007ffff7c80e10`이며, `vmmap`의 libc 매핑 범위에 속한다고 설명함. 후자의 `x/gx` 및 `vmmap` 원문은 채팅에 제출되지 않았으므로 **학생 보고**로 기록.

`printf` GOT 오프셋 `0x4028`도 확인했으며, 사용자는 아직 libc 내부 주소를 가리키지 않는다고 보고. 해당 시점의 구체적 슬롯 값은 미제출.

## 3. GLOB_DAT와 JUMP_SLOT 비교

사용자의 정적 `objdump` 출력에서 `_start`:

```asm
10e4: lea rdi,[rip+0x25f]           # 0x134a (main 인자로 전달)
10eb: call QWORD PTR [rip+0x2ecf]  # 0x3fc0 (__libc_start_main GOT)
```

학생은 `0x134a`를 main ELF 상대 오프셋으로, `0x3fc0`을 `__libc_start_main` GOT 슬롯으로 판단. `starti` 초기 지점은 동적 링커의 `_start`였고, 실행 파일 `_start`까지 진행 후 다음 슬롯 내용을 제출함:

```text
pwndbg> x/gx $rebase(0x3fc0)
0x555555557fc0: 0x00007ffff7c29dc0
```

학생은 libc 매핑 내부임을 확인했다고 보고. PLT의 `JUMP_SLOT` 지연 해결과 달리 `GLOB_DAT` 간접 호출 슬롯은 런타임 시작 전 초기화되는 관계를 교정 후 설명.

## 4. Partial RELRO의 보호 범위

학생 제출한 매핑 발췌:

```text
0x555555558000 0x555555559000 rw-p
```

따라서 `puts@got.plt = 0x555555558008`은 이 쓰기 가능 페이지에 속함. `__libc_start_main@got = 0x555555557fc0`은 해당 페이지 바깥이며, 학생은 읽기 전용 매핑이라고 보고함. 전체 `vmmap` 원문은 미제출.

학생은 Lazy Binding을 위해 일부 `.got.plt` 슬롯에 쓰기 권한이 남고, `GNU_RELRO` 대상의 즉시 바인딩 슬롯은 로딩 후 읽기 전용으로 전환된다고 설명함. 단, 쓰기 가능 매핑 자체가 임의 쓰기 취약점이나 악용 가능성의 증거는 아님.

## 5. CS — Full RELRO와 동적 링킹 순서

처음에 교사의 질문이 모호하여 '이미 기록된 주소를 왜 다시 쓰는가'라고 반문. 이후 학생은 `BIND_NOW`가 활성화된 경우 `main` 전에 GOT에 외부 함수의 주소가 있다는 점을 확인. 동적 링커의 relocation 적용 → 해당 `GNU_RELRO` 영역 읽기 전용 전환 → 프로그램 초기화/진입 순서를 학습. **순서 설명에는 교사 안내가 있었음**.

## 6. 이전 기록 복습 — Day043 / Day050

학생 최초 답변: 정상 함수 진입은 `RSP % 16 = 8`, 이 상태에서 `ret` 하나를 추가하면 `RSP % 16 = 0`. 이 답은 **정상 8에서 추가 ret을 실행한다는 조건**에서는 정확함. 일반적으로 `ret`은 RSP를 8 증가시키므로 모듈로 16의 0과 8을 서로 전환한다.

교사 측 Git 원문 대조: Day043은 정렬 문제를 `ret` **추가**로 해결, Day050은 반대로 `ret` **제거**로 해결. 학생은 이번에는 정렬 수치만 답했으며 과거 기록을 새로 열어 독립적으로 대조한 기록은 없음. 새 조건 반례의 실측은 미실시.

## 7. 장기 기억 — Day156 D+2 지연 복습 (11/22 예정, 11/23 수행)

구조체 `label[12]` 다음 `uint32_t category=0x11223344`인 새 조건. 학생 무힌트 답: 길이 11 정상; 길이 12에서 `label[12]='\\0'`이 인접 category 첫 바이트를 변경; 리틀 엔디언 결과 `0x11223300`; `if (len >= 12) return -1;`로 사전 거부. **4/4 정답**, 처음에 `len[12]`라고 적은 표현은 교사가 `rec.label[12]`로 정정.

## 8. 미완료 및 다음 조치

- 정확한 도구·명령·환경 정보, 보호기법 `checksec` 원문, 실제 `vmmap`, GOT 전후 `x/gx` 전체 원문, 호출 횟수/브레이크포인트 조건과 재현 명령을 사용자의 로컬 로그로 저장하여 Git에 push.
- 정적/동적 비교 결과를 최소 2개 외부 함수로 완성. `printf` 첫 호출 전후 GOT 값은 학생이 직접 전사하여 제출했고 확인되었음. 다만 전체 GDB 명령·실행 환경 원본 로그 및 사용자 커밋은 별도 확인 필요.
- 도움받은 분석을 새 조건에서 무힌트로 설명하는 검증 및 Day043/050 과거 원본 대조·정렬 반례의 문서화.
- 사용자 Git 증거 확인 후에만 **Day157 완료**, 포인터를 Day158로 이동. 교사 작성 이 문서의 커밋을 사용자 로컬 실행 증거와 혼동하지 않음.


## 9. 2026-11-23 — printf 첫 호출 전후 추가 실측 (사용자 제출)

사용자가 `strncmp(..., "echo ", 5)` 분기를 정적 역어셈블로 확인하고, `0x2024`에서 문자열 `"echo "`를 읽어 해당 경로를 스스로 추적. `echo hello` 입력으로 `printf` 경로 도달.

초기에는 `printf` 심볼에 건 브레이크포인트가 libc의 `__printf` 진입을 가리켜, 이미 바인딩이 완료된 것처럼 보였다. 사용자 `x/i $pc`, `bt 3` 실측:

```text
0x555555558028 <printf@got.plt>: 0x00007ffff7c606f0
=> 0x7ffff7c606f0 <__printf>: endbr64
#0 __printf (format=0x55555555602a "echo[%zu]: %s\\n") at ./stdio-common/printf.c:28
#1 0x00005555555552a6 in ?? ()
#2 0x00005555555553c5 in ?? ()
```

이를 첫 호출 자체가 없었다는 증거로 해석하지 않고, libc 함수 진입 시점에는 이미 주소 해결이 끝난다는 점으로 교정. PLT 진입점에 명시적 브레이크를 건 뒤 사용자 보고된 전후 `x/gx` 원문:

```text
# printf@plt 첫 호출 시점
0x555555558028 <printf@got.plt>: 0x0000555555555086
# printf 호출 이후
0x555555558028 <printf@got.plt>: 0x00007ffff7c606f0
```

이는 `printf@plt+6`의 런타임 주소를 가리키던 슬롯이, 바인딩 이후 libc `__printf` 함수 주소로 갱신됨을 뒷받침. 따라서 **두 번째 외부 함수 printf의 지연 바인딩 동적 검증 통과**. 실측 줄은 사용자가 채팅으로 전사한 그대로이며 Pwndbg 전체 원본 세션 로그 파일은 아직 미제출. 추가 개념 시험은 요구하지 않는다.
