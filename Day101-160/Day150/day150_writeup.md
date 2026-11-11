# Day150 — Submit 버퍼 크기 조건 변경과 크래시 원인 분류
2026-11-11 / W05 4/5 / 사용자 로컬 코드·GDB·실행 결과 기반

## 목표와 환경
이전 Day149에서는 `record[48]`으로 57번째 입력 바이트가 Canary의 첫 바이트에 도달했다. 이번에는 사용자 로컬의 Day147 소스를 **별도 Day150 디렉터리에 복사하고 `record[48]`을 `record[64]`로만 바꾼 뒤**, 바이너리의 새 메모리 경계를 찾고 입력값을 바꿔 재검증했다.

- C 소스: `Day147/challenge.c`의 `submit()`에서 `char record[64]`; `receive_bytes(record,112)`는 유지. Full RELRO/Canary/NX/PIE 빌드 스크립트를 복사해 사용했다고 보고.
- 원본 보존: 기존 Day147 소스/바이너리를 수정하지 않고 Day150 로컬 경로에서 실행했다.
- 교사가 GitHub에 추가한 [재현용 소스](experiment_submit64.c)는 Day147 원격 소스에서 **record 선언 한 줄을 수정한 참고 파일**이다. 사용자가 로컬에서 직접 작성한 `challenge.c`의 바이트 완전 일치 여부는 미확인. 로컬 `challenge.c`가 이미 있을 수 있으므로 같은 이름으로 커밋하지 않았다.
- [재현용 빌드 스크립트](build_reproduction.sh)는 명령을 기록하기 위해 추가했으며, 사용자가 직접 실행했다는 증거는 없다.

## 1. 버퍼 크기 변경 후 최초 검증
사용자는 `record[64]`이라면 57바이트 입력은 버퍼 내부에 있으므로 정상 종료할 것으로 예상했다. 반면 `read` 최대 요청 크기 112는 여전히 64를 초과하므로 **취약점 자체는 해결되지 않는다**고 설명했다.

사용자가 `./archive_desk` 또는 GDB에서 전날 만든 57바이트 입력 파일을 사용한 결과, `received: 57`과 정상 종료를 보고했다. 대상 구분을 위해 파일을 복사한 뒤 처음 `./challenge.c`를 실행해 셸의 C 구문 오류가 났고, `run`을 셸에서 실행해 명령을 찾지 못한 상황을 **컴파일 실패와 실행 명령 혼동**으로 교정했다. `./archive_desk`는 셸에서, `run`은 GDB 안에서 사용한다.

## 2. 버퍼와 Canary 위치 — 불필요한 디버깅 단계 제거
처음 `b read`는 Submit이 아닌 `fgets` 내부의 `__GI___libc_read(fd=0,nbytes=1)`에서 중단됐다. 사용자가 백트레이스를 제출한 뒤 `$rdx==0x70` 조건으로 Submit 경로를 구분해 아래 레지스터를 제출했다:

```text
rdi 0x0
rsi 0x7fffffffdf10
rdx 0x70
bt: #0 __GI___libc_read
    #1 0x5555555552c2
    #2 0x5555555553cf
    #3 0x5555555554fe
```

하지만 사용자는 **전체 함수 역어셈블에서 버퍼 포인터를 만드는 명령과 Canary 저장 위치를 확인하면, 호출 프레임을 여러 번 이동하며 계산할 필요가 없다**고 지적했다. 이후 실제 변경 바이너리에서 사용자가 추적·보고한 주소 관계:

```text
record 시작 rbp-0x50  (64B)
padding            (8B)
Canary     rbp-0x8   (8B)
```

Canary 시작까지 거리: `0x50 - 0x8 = 0x48 = 72`바이트. **73번째 입력 바이트**가 Canary의 최하위 주소 바이트에 도달한다고 예측했다. 이 위치 관계는 사용자의 어셈블리 보고에 근거하며 이번 대화에 전체 disassembly 원문은 없었다.

## 3. 같은 길이 73B, 마지막 바이트만 다른 입력
입력 파일은 사용자 로컬에서 교사가 제공한 명령으로 생성했다:

```python
from pathlib import Path
Path("/tmp/day150_null.bin").write_bytes(b"2\n" + b"A"*72 + b"\x00")
Path("/tmp/day150_A.bin").write_bytes(b"2\n" + b"A"*73)
```

두 파일 모두 앞에 메뉴 선택 `2\n`, 뒤에 **Submit 입력 73B**가 있다. 사용자가 실제 제출한 출력:

```text
$ ./archive_desk < /tmp/day150_null.bin
Archive desk / local exercise
1 preview / 2 submit / 3 quit
> record> received: 73
1 preview / 2 submit / 3 quit
>

$ ./archive_desk < /tmp/day150_A.bin
Archive desk / local exercise
1 preview / 2 submit / 3 quit
> record> received: 73
*** stack smashing detected ***: terminated
Aborted (core dumped)
```

동일 길이에서도 첫 Canary 바이트를 `0x00`으로 덮으면 원래 값과 같을 것으로 예상해 검사를 통과하고, `0x41`로 덮으면 Canary 값 불일치로 실패하는 결과와 부합한다. **두 경우 모두 `record[64]` 바깥으로 9바이트가 기록된다.** 정상 종료와 메모리 안전성은 동일한 판단 기준이 아니다. 새 바이너리의 첫 Canary 바이트 GDB 덤프는 제출되지 않았으므로 세부 바이트 대조를 실제로 했다고 주장하지 않는다.

## 4. CS: 구조체 정렬, 내부·끝 Padding
문제:

```c
struct Packet {
    uint8_t type;
    uint32_t length;
    uint8_t flags;
};
```

사용자는 처음에 모른다고 답했지만, 교사의 4바이트 정렬 설명 후 `flags`가 offset8, `sizeof=12`, 끝 padding3B임을 스스로 계산했다. 일반적인 x86-64 GCC ABI의 레이아웃:

| 위치 | 필드 | 길이 |
| --- | --- | --- |
| offset 0 | type | 1B |
| offset 1..3 | 내부 padding | 3B |
| offset 4..7 | length | 4B |
| offset 8 | flags | 1B |
| offset 9..11 | trailing padding | 3B |

사용자 로컬에서 코드 컴파일·실행 후 제공한 결과:
```text
./packet_layout
sizeof: 12
type:   0
length: 4
flags:  8
```
참고용 [packet_layout_reference.c](packet_layout_reference.c)는 교사가 제시했던 코드의 재구성본이다. 기존 로컬의 `packet_layout.c` 내용을 원격에서 직접 읽어 동일성까지 확인한 것은 아니다.

## 5. 보안 2회독: Day118 AFL++ crash dedup
[과거 Day118 write-up](../Day118/write_up.txt)에서는 동일 크래시 주소만으로 동일 취약점이라 단정하지 않고 ASan 오류 종류·손상 객체·누락된 검사·재현 여부를 대조했다.

사용자가 먼저 **서로 다른 입력이 같은 명령어에서 크래시를 낸다면 같은 취약점일 가능성은 있으나 한 취약점이라는 단정은 비약**이라고 지적했다. 조건 변경 예제는 다음 코드의 `memcpy`에서 발생한 세 크래시다:

```c
char payload[16];
memcpy(payload, input, len);
```

| 입력 | len | input_size | 관찰을 가정한 오류 | 구분 |
| --- | ---: | ---: | --- | --- |
| A | 20 | 32 | stack-buffer-overflow WRITE | 목적지 16B를 4B 초과 |
| B | 8 | 4 | heap-buffer-overflow READ | 원본 4B를 4B 초과 |
| C | 24 | 32 | stack-buffer-overflow WRITE | 목적지 16B를 8B 초과 |

사용자는 `A/C`는 payload 크기를 넘어 쓰는 문제, `B`는 input보다 `len`이 커 범위 밖 읽기라고 스스로 설명했다. 이 가정된 예제에서는 **서로 다른 두 원인**(목적지 크기 검사 부재 / 원본 유효 길이 검사 부재)으로 분류하는 것이 타당하다. 세 입력이 발견됐으므로 세 취약점이라는 뜻은 아니며, 한 줄의 `memcpy`에서 발생했더라도 하나로 단정하지 않는다. 이 복습은 **손 추적**, 실제 ASan 신규 실행은 아니다.

## 6. 도움과 확인한 범위
- **사용자 독립 판단:** 새 버퍼의 57B 정상 예측, 기존 112B read의 문제 지속, disassembly 기준 오프셋 추적 방식을 제안, 73번째 NUL 가설, GDB/실제 실행 결과 제출, CS 학습 후 계산, 복습 A/C와 B 원인 구분.
- **교사 도움:** 파일 복사·컴파일 절차, `read` 조건 breakpoint, 정렬 개념, 파일 생성 코드, ASan 가상 사례와 질문. 사용자 오류였던 소스 실행과 셸/GDB 명령 혼동을 교정.
- **미제출:** 사용자 로컬 바이너리/전체 디버깅 로그/컴파일 로그/해시·도구 버전/실제 소요시간. 새 `record[64]` Canary 덤프와 당일 AST/ASan 별도 실행은 없음.
- **계획 대조 제한:** 현재 GitHub `hanyang-research-plan-2027.xlsx`의 Day149/150 셀은 연결 도구가 binary UTF-8 오류로 반환하여 아직 대조하지 못했다. 본 보고서는 **실제로 진행한 학습 과제 및 증거**에 대한 결과이며, 원래 엑셀의 추가 필수 항목이 확인되면 보완한다.

## Git 경로
- [사용자 증거 발췌](day150_user_observations.md)
- [변경 바이너리 참고 소스](experiment_submit64.c)
- [구조체 정렬 참고 소스](packet_layout_reference.c)
- [재현 빌드 스크립트](build_reproduction.sh)
- [Day150 README](README.md)
