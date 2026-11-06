# Day147 — archive_desk 독립 분석 진단: Format String과 Submit Stack BOF
2026-11-05~06 / W05 1/5 / PwnLab 중심 독립 Pwn 진단

## 목표·근거·환경
계획의 Day147 목표는 **새 로컬 CTF 바이너리를 소스/해설보다 먼저 관찰하여 보호기법, 메뉴, 입력 경로와 취약점의 근거를 제시하는 것**이다. Exploit 완성은 이번 Day의 필수 조건이 아니다.

- 대상: `Day101-160/Day147/archive_desk` (사용자가 로컬 WSL에서 GDB/pwndbg 실행).
- 재현 빌드: `sh Day101-160/Day147/build.sh`; gcc `-O0 -fstack-protector-all -fPIE -pie -Wl,-z,relro,-z,now -Wl,-z,noexecstack -s`.
- 사용자 보고 보호기법: **stripped / Full RELRO / Canary / NX / PIE**.
- 메뉴: `1 preview / 2 submit / 3 quit`. 메뉴 입력은 `fgets(buf,0x10,stdin)` 후 `strtol(...,10)` 경로로 분석했다.
- 이 문서의 역어셈블·메모리 맵·출력은 **사용자가 채팅에 붙여 넣은 발췌**이며 전체 원본 실행 로그나 교사 재실행이 아니다. 자세한 증거는 [day147_user_observations.txt](day147_user_observations.txt)에 보존한다.

## 1. Preview: 입력 소비 및 Format String 확인
Preview에서 `fgets([rbp-0x40],0x30,stdin)`을 확인했다. 이는 최대 **47바이트 데이터 + 종료 NUL**을 저장하는 호출이다. 이어 같은 `[rbp-0x40]` 버퍼가 `printf@plt`의 **RDI (형식 문자열)**로 전달된다:

```asm
lea rax,[rbp-0x40]
mov rdi,rax
mov eax,0
call printf@plt
```

이것은 `printf("%s",buf)`가 아니라 `printf(buf)`와 대응하는 취약한 호출 경로다. 사용자가 `%p`에서 주소처럼 보이는 값을 관찰하고 `[%p][%15p]`에서 `[0x628d2c2f4031][     0xffffffff]`를 확인했다. 이때 `%15p`의 15는 **필드 최소 너비**이며, 위치 지정 `%15$p`와 다르다(교사 설명 후 교정).

이후 별도 GDB 실행에서 `[%1$p]...[%5$p]` 출력과 실제 레지스터를 대응시켰다:

| 지정자 | 후보 위치(SysV AMD64의 가변 인자) | GDB 값 / Preview 출력 |
| --- | --- | --- |
| `%1$p` | RSI | `0x555555556031` |
| `%2$p` | RDX | `0xffffffff` |
| `%3$p` | RCX | `0x7ffff7d148c7` |
| `%4$p` | R8 | `0x9` |
| `%5$p` | R9 | `(nil)` / 0 |

형식 문자열 외 추가 인자를 제공하지 않는 `printf(buf)`에서 값을 읽는 것은 C 차원에서 정의되지 않은 동작이다. **이 환경에서의 실측 GDB 값이 출력과 일치했다**는 것으로 주장을 제한한다. 독립 실행과 GDB 실행의 ASLR 주소를 동일한 실행 값처럼 합치지 않는다.

Preview에 47개 `A`와 뒤따르는 `3`을 입력한 뒤 `unknown choice`와 메뉴 재출력이 관찰됐다. 이전 입력이 다음 메뉴 읽기에 남았을 가능성을 제시하지만 잔류 바이트의 정확한 소비 순서는 미확정이며 악용 성공으로 기록하지 않는다.

## 2. 누출 주소의 소속·베이스 계산
사용자 `info proc mappings` 및 `x/s` / `x/5i` 관찰:

- `0x555555556031` → `"preview: "`; `archive_desk`의 `r--p` 구간(시작 `0x555555556000`, 파일 오프셋 `0x2000`). ELF 이미지 베이스 `0x555555554000`, 누출 문자열의 이미지 기준 오프셋 **`0x2031`**.
- `0x7ffff7d148c7` → `__GI___libc_write+23` (`cmp rax,0xfffffffffffff000`); libc의 `r-xp` 구간. libc 매핑의 offset 0 시작은 `0x7ffff7c00000`, 누출 주소의 베이스 기준 오프셋 **`0x1148c7`**.

사용자는 PIE 베이스 `0x555555554000`을 정확히 계산했다. libc의 실행 구간 시작 `0x7ffff7c28000`을 처음에는 베이스/오프셋과 혼동했고 교정 후 `leak - image_base` 의미를 확인했다(산술값은 교사가 계산). **매핑 시작/파일 오프셋/이미지 베이스는 구분**해야 한다.

## 3. Submit: 호출자 스택에 쓰는 read로 인한 BOF
사용자 역어셈블에서 Submit 함수 `0x555555555380`은 `sub rsp,0x50`과 `[rbp-0x8]` Canary 저장 뒤 다음과 같이 헬퍼를 호출한다:

```asm
lea rax,[rbp-0x40]    ; Submit이 제공한 목적지 포인터
mov esi,0x70          ; 최대 요청 112
mov rdi,rax
call 0x55555555528a  ; read wrapper
mov QWORD PTR [rbp-0x48],rax ; 반환값 저장, 버퍼 시작 아님
```

헬퍼(`0x28a`)는 RDI/RSI 인자를 각각 보관했다가 `read(0, caller_buffer, 0x70)`으로 전달한다. Submit 프레임 안의 버퍼 시작 주소는 `[rbp-0x40]`, Canary 저장 주소는 `[rbp-0x8]`이다.

| Submit 버퍼 시작 기준 | 거리 |
| --- | --- |
| Canary | `0x38` = 56B |
| Saved RBP | `0x40` = 64B |
| Saved return address | `0x48` = 72B |
| `read`의 최대 요청 크기 | `0x70` = 112B |

**112B 요청 > Canary까지 56B**이고, 실제 긴 `a` 입력으로 `__stack_chk_fail`을 관찰했다. `read`의 반환 길이는 실행 시 상황에 따라 달라지므로 항상 112B가 읽혔다고 기록하지 않는다. 배열의 원본 C 선언 크기 역시 이 어셈블리만으로 확정하지 않는다.

```asm
0x555555555406: mov rax,QWORD PTR [rbp-0x8]
0x55555555540a: sub rax,QWORD PTR fs:0x28
0x555555555413: je  0x55555555541a
0x555555555415: call __stack_chk_fail@plt
0x55555555541a: leave
```

GDB `bt`의 `#0 __stack_chk_fail`, `#1 0x55555555541a`는 호출한 Submit의 종료 검사로 이어진다. `0x541a`는 fail 호출 **직후의 복귀 위치**이며 call 명령 자체는 `0x5415`다. `#2` 이하 `0x6161616161616161`은 입력 `a`에 의한 스택 오염을 시사하지만 오염된 백트레이스를 정상적 `ret` 성공이나 RIP 제어 성공으로 간주하지 않는다.

헬퍼 자신의 Canary 검사도 있지만, 쓰기 대상은 **상위 호출자인 Submit이 전달한 버퍼**다. 별도의 아래쪽 스택 프레임에 있는 헬퍼의 Canary가 보존될 수 있고, 헬퍼가 반환한 뒤 Submit Canary가 실패할 수 있다. 사용자는 처음 `[rbp-0x48]`을 버퍼 시작이라 해석했다가 호출 인자 추적으로 `[rbp-0x40]`으로 수정했고, 기존 프레임을 '덮는' 것이 아니라 추가 프레임을 쌓는 구조를 교정했다.

## 4. CS — signed/unsigned와 입력 계약
교사가 제시한 설명용 코드(`int32_t len`, `uint8_t payload[32]`, `memcpy(payload,input+4,(size_t)len)`)에서 다음을 문답으로 확인했다.

1. `len=-1`은 `len>32` 검사를 통과할 수 있고, 64비트 `size_t`로 변환 시 `SIZE_MAX`가 되어 잘못된 대량 복사를 요청한다(UB, 실제 크래시 형태는 미측정).
2. `input_size=20, len=20`은 목적지 32B 안에 들어가지만 헤더 4B를 뺀 원본 데이터는 16B여서 **4B 범위 밖 읽기**가 된다. 사용자는 처음 놓쳤다가 헤더를 짚은 후 스스로 수정했다.
3. 사용자 제안: `len<0 || len>32` 및 `len>input_size-4`. 최종 안전 순서에서는 **`input_size<4`을 먼저 검사**하고 `len`의 비음수 조건을 확인한 뒤 부호 없는 크기로 비교한다. 아래는 개념상 수정안이며 Day147 바이너리의 수정 실험은 아니다.

```c
if (input_size < 4) return -1;
int32_t len;
memcpy(&len, input, sizeof(len));
uint8_t payload[32];
if (len < 0 || (size_t)len > sizeof(payload)) return -1;
if ((size_t)len > input_size - 4) return -1;
memcpy(payload, input + 4, (size_t)len);
```

## 5. 보안 2회독 — Day085 Tcache Poisoning
과거 [Day085 힙 복합 실습](../../Day040-100/Day085/write_up.txt)의 **tcache count=1** 실패를 다시 회상했다. `A→TARGET`로 `next`를 변조해도 첫 `malloc`이 `A`를 반환하면서 count가 0이면 두 번째 할당은 tcache에서 TARGET을 회수하지 않는다. 초기 count=2일 때는 첫 반환 후 count가 1이므로 다음 할당에서 TARGET 후보를 사용할 수 있다(정렬·glibc 검증 전제 필요). 사용자는 처음 기억나지 않았으나 **count가 0이면 비어 있다고 판단한다**고 힌트 후 답했다. 과거 write-up 대조이며, 오늘 힙 대상의 재실행/무힌트 재풀이로 기록하지 않는다.

## 6. 검증 수준과 미완료 범위
- **사용자 직접 관찰:** stripped/보호기법, 메뉴, Preview 형식 문자열·레지스터 대조, 메모리 매핑·문자열·libc 명령어, Submit 헬퍼 `read` 인자, Canary 검사·실패 백트레이스.
- **교사 도움:** 위치지정자 `%N$p`, GDB 명령/순서, ELF 매핑 해석과 libc 오프셋 산술, 버퍼 시작 재검토, 호출 프레임 설명, CS 반례와 tcache count 힌트. 무힌트 완전 독립 exploit으로 평가하지 않는다.
- **확인하지 않음:** Canary 자체의 Format String 누출 및 대조, ROP/제어 흐름 탈취, 임의 코드 실행, 원본 source 기반 배열 선언 크기, 정확한 `read` 반환값/전체 GDB transcript/해시, 독립 조건변경 exploit 테스트. 새 CS 예제·Day085 재실행 없음.
- **완료 범위:** W05 1/5에서 계획한 *기초 입력 경로·보호기법 및 취약점 근거 진단*, CS·보안 복습과 기록. 다음 Day의 독립 분석 능력까지 확인한 것은 아니다.

다음: **Day148 / W05 2/5**, 계획표의 독립 Pwn 진단 다음 회차. 이전 풀이를 답으로 암기하기보다 새 조건의 근거를 먼저 찾아가며 힌트 의존성을 명시한다.
