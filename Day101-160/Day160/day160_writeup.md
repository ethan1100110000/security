# Day160 — FINAL W07 4/5 — 보호기법 통제 실험 (진행 중)

- 학습 시작: 2026-11-26 (KST)
- **최종 상태: Day160 / FINAL W07 4/5 완료 (2026-11-26).** 4변형 보호기법 대조, ELF 정적 근거, 학생 로컬 원본 실행 로그, CS·보안·Day158 D+2 복습 확인.
- 실험 설계: 동일 `lab160.c`에서 기본 Full RELRO/PIE ON/NX ON과 각각 한 설정만 바꾼 Partial RELRO, PIE OFF, NX OFF 변형을 비교. 세부 과제는 [README](README.md)를 따른다.

## 1. 실행 전 학생의 초기 가설 — 채팅 원문

질문: ① Full→Partial RELRO에서 GOT 쓰기 권한, ② PIE OFF + ASLR ON에서 `main` 주소 변화, ③ NX OFF에서 스택 코드 실행과 ROP 필요성.

학생 답변 원문: `1. write 가능 2. 안바뀜 3. 실행은 가능 rop필요할 수 있음`.

- 잠정 평가: 세 방향 모두 타당하나 ①에서 모든 GOT 슬롯이 writable인 것은 아니며 일반적으로 lazy binding을 사용하는 `.got.plt`와 RELRO 보호된 `.got`를 구별해야 함. ②는 일반적인 Linux non-PIE main 코드에 대한 예측으로, libc/stack ASLR 비활성화를 뜻하지 않음. ③은 스택 실행 권한이라는 한 조건만 바뀐 것이며, 임의 분기 제어나 제약 조건을 해결했다고 뜻하지 않음.
- 사전 예측 시점에는 실측을 제출하지 않았고, 아래 본문에 이후 실제 학생 측 관찰 및 최종 Git 원본 검증을 시간순으로 구분하여 기록했다.

## 2. 첫 번째 학생 측 정적 관찰 (2026-11-26)

학생은 기본형 `bin/lab_full_pie_nx`에 대하여 `checksec`의 보호기법이 모두 예상대로 나왔고, `GNU_STACK`이 `RW`이며 실행 권한이 없다고 **요약 보고**했다. **이후 Git 원본 로그**에서 `Full RELRO`, `Canary found`, `NX enabled`, `PIE enabled`, `GNU_STACK RW`가 직접 검증됐다.

학생이 직접 제출한 `readelf -dW bin/lab_full_pie_nx | grep -E 'BIND_NOW|FLAGS'` 출력:

```text
0x000000000000001e (FLAGS)              BIND_NOW
0x000000006ffffffb (FLAGS_1)            Flags: NOW PIE
```

`DT_FLAGS`의 `BIND_NOW` 및 `DT_FLAGS_1`의 `NOW`는 함수 재배치의 즉시 해결을 뜻하고, `PIE` 비트는 PIE 실행 파일이라는 동적 플래그다. 이는 Full RELRO 동작과 일관되지만 **해당 두 줄만으로 GOT 페이지의 읽기 전용 매핑을 직접 관찰한 것은 아님**. 다음으로 `GNU_RELRO` 세그먼트 존재와 Partial 비교 및 동적 GOT 자기값 쓰기를 확인한다.

## 3. Partial RELRO와 puts@plt 관찰 — 학생 원문

학생이 제출한 `readelf -dW bin/lab_partial_pie_nx | grep -E 'BIND_NOW|FLAGS'`:

```text
0x000000006ffffffb (FLAGS_1)            Flags: PIE
```

따라서 Full 변형에서 나타났던 `BIND_NOW`/`NOW`가 없고, 즉시 바인딩 요구가 관찰되지 않음.

학생이 제출한 `objdump -d -M intel bin/lab_partial_pie_nx | grep -A 5 '<puts@plt>:'`:

```asm
0000000000001140 <puts@plt>:
    1140: f3 0f 1e fa           endbr64
    1144: f2 ff 25 dd 2e 00 00 bnd jmp QWORD PTR [rip+0x2edd] # 4028 <puts@GLIBC_2.2.5>
    114b: 0f 1f 44 00 00     nop DWORD PTR [rax+rax*1+0x0]
```

`0x4028`은 `puts`의 GOT 슬롯 **ELF 이미지 상대 주소**로, 이 명령은 런타임 해당 슬롯 안의 8바이트 포인터로 간접 점프한다. 이 PLT 엔트리에는 직접 `push`가 보이지 않으므로 예전 Day157의 전형적인 `puts@plt+6` 형태를 이 바이너리에 기계적으로 대입할 수 없음. 별도 `.plt` 해석 스텁과 `R_X86_64_JUMP_SLOT` 및 실제 첫 호출 전 GOT 값을 추적할 예정; 아직 런타임 GOT 값·메모리 권한은 관측하지 않았음.


### 첫 호출 전 GOT 런타임 관찰 (학생 직접 제출)

학생이 `start` 이후 확인한 값:

```text
pwndbg> x/gx $rebase(0x4028)
0x555555558028 <puts@got.plt>:  0x0000555555555050
```

`0x555555555050`은 이 PIE 실행 파일의 PLT 코드 쪽 주소로 추정되며, libc `puts`의 직접 주소가 아직 기록되지 않았다는 가설을 뒷받침한다. 이번 `puts@plt`는 `.plt.sec`의 `0x1140`에 `endbr64; bnd jmp [rip+...]` 형태로 존재하므로 이전 Day157의 `puts@plt+6` 공식을 그대로 적용하지 않는다. **GOT 값의 정확한 디스어셈블리와 첫 `puts` 호출 후 값 변화는 추가로 관찰해야 함**. 원본 출력은 대화에 제출됐으며 전체 학생 로그 Git 제출은 아직 확인 안 됨.

### 런타임 PLT 지연 해결 스텁 확인 (학생 직접 GDB 실측)

사용자 GDB 원문:

```text
pwndbg> x/6i 0x555555555050
   0x555555555050:      endbr64
   0x555555555054:      push   0x2
   0x555555555059:      bnd jmp 0x555555555020
   0x55555555505f:      nop
   0x555555555060:      endbr64
   0x555555555064:      push   0x3
```

`puts@got.plt`의 최초 내용 `0x555555555050`는 현재 바이너리의 PLT 재배치 스텁 `PIE+0x1050`임이 정적/동적으로 확인됐다. `push 0x2`는 lazy binding resolver가 사용할 PLT 재배치 인덱스이며, `bnd jmp 0x...5020`는 공통 PLT0 주소 해결 진입으로 이어짐. `endbr64; push 0x3`는 **그다음 인접 PLT 스텁**이므로 `puts` 호출 경로로 연결했다고 주장하면 안 됨. 다음으로 호출 전후 GOT 값의 변화를 실측하면 함수 주소 해결까지 확인 가능.

### puts 호출 후 GOT 갱신 (학생 직접 GDB 실측)

학생이 이전에 `puts@got.plt`에 `0x0000555555555050` (PLT 스텁)이 있음을 확인했고, GDB에서 `marker()` 호출을 수행하라는 과제 후 다음 값을 제출:

```text
pwndbg> x/gx $rebase(0x4028)
0x555555558028 <puts@got.plt>:  0x00007ffff7c80e10
```

학생 해석: "이제는 got 슬롯에 puts의 주소가 있음". 주소가 실행 파일 내의 PLT 스텁에서 libc 주소 범위로 변화한 것으로 설명됨. 사용자는 전후 값과 동일 슬롯을 제시했으며 실제 `call marker()` 출력은 별도로 공유하지 않았으므로, **호출 전후 차이와 사용자의 실행 설명에 근거한 판정**이고 다른 메모리 맵의 독립 `vmmap` 원문은 아직 없음. 이 관찰은 lazy binding의 GOT 갱신을 뒷받침하지만 **GOT 슬롯의 쓰기 가능성 자체를 측정한 것은 아니며** 다음 `got` 프로브로 따로 검증해야 함.

### Full/Partial GOT relocation offset 비교 (학생 정적 관찰)

학생 직접 제출 `nm -n` 및 `readelf -rW` 원문:

```text
# bin/lab_full_pie_nx
0000000000001629 T main
0000000000003f78  0000000500000007 R_X86_64_JUMP_SLOT     0000000000000000 puts@GLIBC_2.2.5 + 0

# bin/lab_partial_pie_nx
0000000000001629 T main
0000000000004028  0000000500000007 R_X86_64_JUMP_SLOT     0000000000000000 puts@GLIBC_2.2.5 + 0
```

두 빌드 모두 `main` ELF 심볼 값은 `0x1629`, `puts`의 JUMP_SLOT 오프셋은 각각 Full=`0x3f78`, Partial=`0x4028`로 달랐다. 학생이 "둘이 달라"라고 **독립 발견**. 이는 링킹 결과의 GOT 슬롯 배치가 달라졌음을 입증하지만, 오프셋 차이만으로 Full의 읽기 전용 보호를 확정할 수 없다. `got 0x1629 0x3f78` / `got 0x1629 0x4028` 실행의 실측 결과 대기.

### 학생의 GOT 동일값 쓰기 결과 보고 (2026-11-26)

다음 두 명령을 수행하도록 안내한 뒤 학생이 **요약 보고**함:

```bash
./bin/lab_full_pie_nx got 0x1629 0x3f78
./bin/lab_partial_pie_nx got 0x1629 0x4028
```

학생 원문: `예상이 맞아 full 에서는 오류가 발생 partial에선 write가 성공하고 정상종료`.

- Full: GOT 동일값 write 시 오류를 관찰했다고 보고. 당시 **실제 자식 signal 번호/원시 출력은 미제출**이었으나, 이후 Git 원본에서 `got_same_value_write: signal 11 (Segmentation fault)`로 직접 확인.
- Partial: GOT 동일값 write 성공 및 정상 종료를 보고.
- 두 결과는 사전 가설과 부합한다. 다만 이 실험은 쓰기 접근성만 검사하며 임의 쓰기 취약점 존재 여부와 무관하다. 추후 로컬 원본 로그가 있으면 종료 상태를 직접 대조할 것.

### PIE ON/OFF 동적 주소 비교 — 학생 원문 6회 (2026-11-26)

실행 명령: `for binary in lab_full_pie_nx lab_full_nopie_nx; do for i in 1 2 3; do ./bin/$binary addr; done; done`.

```text
main=0x58334fb16629 marker=0x58334fb162e9
main=0x5d63de6c7629 marker=0x5d63de6c72e9
main=0x654a22817629 marker=0x654a228172e9
main=0x4015d8 marker=0x4012d6
main=0x4015d8 marker=0x4012d6
main=0x4015d8 marker=0x4012d6
```

- PIE ON `main` 값 3개가 모두 다르며 `main - marker = 0x340`은 3회 일정. `main` ELF 오프셋 `0x1629`를 적용한 각 실행 PIE base는 `0x58334fb15000`, `0x5d63de6c6000`, `0x654a22816000`이다.
- PIE OFF `main=0x4015d8`, `marker=0x4012d6`이 3회 고정이며 `main-marker=0x302`. **두 빌드 간 상대 오프셋 자체는 `0x340` vs `0x302`로 다르다.** 이 차이는 서로 다른 빌드의 code layout 차이로, 같은 빌드 내 오프셋 불변과 모순되지 않는다.
- 이 출력은 3회 실행에서의 관측이지, 모든 실행에서 반드시 주소 변화 또는 불변을 보장한다고 일반화하지 않는다. PIE OFF여도 libc·stack ASLR까지 꺼지는 것은 아님.

### NX ON/OFF 스택 RET 실행 실측 (학생 직접 제출, 2026-11-26)

실행과 제출된 결과:

```text
bash-5.1$ ./bin/lab_full_pie_nx stack
stack_probe: signal 11 (Segmentation fault)
bash-5.1$ ./bin/lab_full_pie_execstack stack
stack_probe: exit 0
```

- 동일 C 소스의 스택 1바이트 `0xc3`(x86-64 `ret`) 실행 시도. NX ON은 자식 프로세스의 `SIGSEGV`(11), execstack 빌드에서는 정상 종료(exit 0). 학생은 실행 전에 NX OFF에서 스택 코드 실행이 가능하다고 예측했고 결과가 부합함을 보고.
- 차이는 `-Wl,-z,noexecstack` vs `-Wl,-z,execstack` 조건에 의한 스택 실행 권한 변화와 일관됨. Git 원본 로그에서 해당 빌드의 `GNU_STACK RWE`와 NX ON 기본형 `GNU_STACK RW`가 직접 확인됨. 실측은 스택 실행 페이지 권한을 시험한 것이며 ROP 가능/불가능, 취약점의 존재나 exploit 완성을 입증한 것은 아님.

### NX 정적 근거 및 Full RELRO CS 이해 점검 (학생 보고)

학생은 `readelf -lW` 명령으로 기본 `lab_full_pie_nx`의 `GNU_STACK` 권한이 `RW`, `lab_full_pie_execstack`의 권한이 `RWE`임을 확인했다고 보고함. **이후 Git 원본에서도 두 명령과 출력의 일치를 확인함**. 실제 NX ON에서 `signal 11`, execstack에서는 `exit 0`와 일치하므로 **정적 플래그와 동적 실행 결과 교차 비교**됨.

CS 질문 "Full RELRO인데 스택 반환 주소를 덮을 취약점이 있으면 ROP가 불가능해지는가?"에 학생이 "Full RELRO여도 스택의 반환주소는 덮을 수 있음; Full RELRO는 GOT 테이블만 read only로 보호"라고 답함. **반환 주소와 GOT 보호 대상 구분은 정답.** 다만 Full RELRO는 GOT만이 아니라 `PT_GNU_RELRO`로 지정된 여러 데이터 영역을 relocation 후 읽기 전용으로 만들 수 있으므로 "GOT만"이라는 표현은 교정 필요. ROP는 별도의 반환주소 덮어쓰기 취약점과 실행 가능한 코드 조각을 활용한다는 구분 유지.

### Day158 D+2 지연복습 — GOT 누출 출처 판별 (학생 무힌트 답변, 2026-11-26)

변경 조건: Partial RELRO 빌드의 `puts@got.plt` 호출 전 `0x555555555050`(ELF PLT 스텁), 호출 후 `0x7ffff7c80e10`(libc puts) 중 어떤 것을 libc leak으로 쓸지 판단. 가정한 libc `puts` 심볼 오프셋 `0x80e10`.

학생의 최초 답변(원문): `1. 두번째 2. 0x7ffff7c00000 3. 첫번째주소로 구한 libc base로 한 모든 주소 계산이 잘못된거니까 다 망함`.

판정: 세 가지 정답. `0x7ffff7c80e10 - 0x80e10 = 0x7ffff7c00000`. 첫 GOT 값은 본 ELF의 PLT 스텁이므로 libc 베이스 계산에 사용할 수 없고, 이를 libc 주소로 오해할 경우 해당 베이스로부터 파생한 libc 심볼 주소 전체가 잘못됨. 이는 기존 Day158의 누출 출처/매핑 신뢰도 학습을 새 조건에 적용한 것으로 **독립 회상 통과**. 단, 해당 libc puts offset은 이번 문제에서 주어진 가정이며 별도의 실제 libc 심볼 표 확인을 학생에게 요구하지 않음.

### Day043/050 변경 조건 — ROP 슬롯 소비 독립 계산 (2026-11-26)

새 문제: 첫 가젯 진입 `RSP%16=0`. Gadget 1은 `pop rbx; pop r12; ret` (8B × 3 = 24B), Gadget 2는 `pop rdi; ret` (8B × 2 = 16B), 목표 함수 진입 필요 `RSP%16=8`.

사용자 **힌트 없는 최초 답** 원문: `1. 8 2. 8 3. 필요없음`. 세 답 모두 정확함. 첫 가젯 종료 후 `(0+24)%16=8`, 두 번째 종료 후 `(8+16)%16=8`. 따라서 별도 정렬용 `ret`을 추가하면 도리어 `RSP%16=0`이 되어 조건을 어기게 됨. Day159 복습 당시에는 정렬 변화의 숫자 계산에 교사의 도움이 필요했으나, 이번 변경조건에서는 무힌트로 최종 모듈러 결과와 추가 ret 불필요를 맞힘. **Day043·050 변경 복습 통과**.

## 4. 학생 원본 실행 로그 Git 검증 및 완료 판정 (2026-11-26)

학생 직접 Git 커밋: `31380fde92d53b855ddb06ae55cb67a2b6906139` (`study: Day160 protection comparison evidence`), 파일 [day160_session.log](day160_session.log), Git blob `64ad52375320ddd10f9adffa89a3a2051ee05346`. 로그는 `script` 원시 터미널 기록이며 ANSI escape sequence를 포함한다. 시작 `2026-11-26 15:56:01+09:00`, 종료 `16:38:01+09:00`, `COMMAND_EXIT_CODE=0`. 채팅 답과 구별되는 **학생 실행 기록**으로 검토함.

원시 로그에서 직접 확인한 핵심 출력:

```text
# Full RELRO ELF
RELRO: Full RELRO
Stack: Canary found
NX: NX enabled
PIE: PIE enabled
GNU_STACK ... RW
(FLAGS) BIND_NOW
(FLAGS_1) Flags: NOW PIE
GNU_RELRO ... R

# Partial RELRO: lazy puts binding
(FLAGS_1) Flags: PIE
puts@got.plt before call marker(): 0x0000555555555050
(gdb) call (void) marker()
LOCAL_MARKER
puts@got.plt after call: 0x00007ffff7c80e10

# GOT same-value write probes
./bin/lab_full_pie_nx got 0x1629 0x3f78
got_same_value_write: signal 11 (Segmentation fault)
./bin/lab_partial_pie_nx got 0x1629 0x4028
got_same_value_write: exit 0

# NX probes
./bin/lab_full_pie_nx stack
stack_probe: signal 11 (Segmentation fault)
./bin/lab_full_pie_execstack stack
stack_probe: exit 0

# GNU_STACK program headers
lab_full_pie_nx: RW
lab_full_pie_execstack: RWE
```

GDB 원본에는 `start`, `x/gx $rebase(0x4028)`, `x/6i 0x555555555050`, `call (void) marker()`, 호출 후 GOT 재확인까지 기록됨. GDB 종료 시 `No selected thread` Python 예외가 출력되었지만 **그 이전의 GOT 관찰을 무효화하는 증거는 아니며**, 전체 세션 로그는 정상적으로 닫힘. PIE ON 3회와 OFF 3회 관찰이 최소 두 세트 기록됐고, 3회 실행값 및 함수 간 차이는 위 절과 일치함.

**증거 구분:** 메인 통제 실험의 정적·동적 관찰은 Git 원본으로 교차 검증. CS·Day043/050 변경 조건 및 Day158 D+2 무힌트 답변은 사용자와의 **채팅 답변 근거**로 남아 있고 원본 터미널 로그의 수치 실험으로 간주하지 않음. 본 lab은 제공된 교육용 소스 사용 통제 실험이며, 새로운 취약점을 독립 발견하거나 실전 exploit을 완성한 것으로 기록하지 않는다.

**최종 판정: Day160 / FINAL W07 4/5 완료.** 다음은 **Day161 / FINAL W07 5/5**. 같은 실험의 불필요한 재실행 요구 없이 새 일일 과제로 이동한다.
