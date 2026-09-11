# Day120 - Toy Parser Fuzzing Write-up

## 1. Target과 입력 구조

분석 대상은 `parse_record(const uint8_t *input, size_t input_size)` 함수다. 입력은 다음 레코드 형식을 사용한다.

| Offset | 크기 | 필드 | 조건 |
|---:|---:|---|---|
| 0 | 4 | Magic | ASCII `TPAR` |
| 4 | 1 | Version | `1` |
| 5 | 1 | Command | `1` 또는 `2` |
| 6 | 1 | Length | Payload 복사 길이 |
| 7 | Length | Payload | 실제 데이터 |

Parser는 `START → MAGIC → VERSION → COMMAND → LENGTH → PAYLOAD → DONE` 순서로 상태를 진행한다. 원본 코드는 `length <= input_size - 7`에 해당하는 검사를 수행해 원본 입력 범위 밖 읽기는 막았지만, `length <= sizeof(payload)` 검사는 수행하지 않았다.

위험한 연산은 다음과 같다.

```c
char payload[16] = {0};
memcpy(payload, input + 7, length);
```

따라서 입력에 Length만큼의 데이터가 실제로 존재하더라도 Length가 16보다 크면 스택의 `payload[16]`을 넘는 쓰기가 발생한다.

취약한 CLASSIC 바이너리의 SHA-256은 다음과 같다.

```text
d4b515a8dd88dad7ba3ca4bdd6b3fb97f7696ae3d80eb4cb808f679082968aff  Day101-160/Day116/day116_classic
```

분석에 사용한 Day116~117 저장소 상태는 commit `d93a5cc2b52f5be40e399c2759481121a7d6f2b7`, 패치와 최종 검증 상태는 commit `5c359838f9f96b404328d2703efb7661a5cb8fb4`에 보존돼 있다.

## 2. Harness

stdin harness는 `read(STDIN_FILENO, input, sizeof(input))`으로 최대 256바이트의 raw input을 읽는다. 이후 버퍼 주소와 실제로 읽은 길이를 parser에 전달한다.

```c
ssize_t received = read(STDIN_FILENO, input, sizeof(input));
return parse_record(input, (size_t)received) == 0 ? 0 : 1;
```

두 번째 인자로 `sizeof(input)`이나 `strlen(input)`을 사용하지 않고 `read()` 반환값을 사용했다. `sizeof(input)`은 실제 입력 길이가 아니라 버퍼 용량이며, `strlen()`은 바이너리 입력 중간의 NUL 바이트에서 멈추거나 NUL 종료가 없을 때 범위 밖 읽기를 일으킬 수 있기 때문이다.

Day115에서는 file harness도 만들었다. AFL++ 명령의 `@@`는 현재 변이 입력 파일의 경로로 치환되며, file harness는 해당 파일을 `open/read`한 뒤 같은 `parse_record()`를 호출한다. stdin harness에는 `@@`가 필요 없다.

Harness는 전체 프로그램의 불필요한 초기화를 거치지 않고 목표 함수에 빠르고 결정적으로 도달하게 한다. 다만 실제 caller의 필수 사전조건을 생략하면 실제 프로그램에서는 도달할 수 없는 crash를 만들 수 있으므로, 단순한 실행 편의 코드가 아니라 실제 data flow와 precondition을 보존해야 한다.

## 3. Fuzz 실행 환경과 결과

실제 crash 수집에는 AFL++ CLASSIC coverage 계측과 ASan을 함께 적용한 빌드를 사용했다. CLASSIC은 edge coverage를 기록하고, ASan은 범위 밖 읽기·쓰기와 같은 메모리 오류를 잘못된 접근 지점에서 탐지한다.

동일한 설정을 재구성하는 빌드 예시는 다음과 같다.

```bash
cd Day101-160/Day116

AFL_LLVM_INSTRUMENT=CLASSIC AFL_USE_ASAN=1 \
afl-clang-fast -O0 -g -fno-omit-frame-pointer \
  day116_parser.c day116_stdin_harness.c \
  -o day116_fuzz_asan
```

실제 `fuzzer_stats`에 보존된 실행 명령은 다음과 같다.

```bash
afl-fuzz -m none -V 120 -i in -o out_classic_asan -- ./day116_fuzz_asan
```

| 항목 | 값 | 의미 |
|---|---:|---|
| AFL++ version | 4.00c | 퍼저 버전 |
| run_time | 120초 | 실행 시간 |
| execs_done | 36,299 | 총 target 실행 횟수 |
| execs_per_sec | 302.48 | 평균 초당 실행 횟수 |
| corpus_count | 12 | 새로운 coverage 또는 hit-count 변화를 만들어 보존된 입력 |
| stability | 100.00% | 같은 입력에서 coverage가 일관됨 |
| saved_crashes | 1 | 저장된 crash 입력 수 |
| saved_hangs | 0 | 저장된 hang 수 |

`stability=100%`는 프로그램이 crash하지 않았거나 메모리 안전하다는 의미가 아니다. 같은 입력을 반복했을 때 coverage가 일정했다는 뜻이다. 또한 `saved_crashes`는 입력 파일 수이지 취약점 수가 아니므로 단독 재현과 root-cause 분류 전에는 취약점 개수로 해석하지 않았다.

초기 PCGUARD 빌드에서는 9개의 crash가 저장됐지만, 이는 이후 분석에서 target 취약점이 아닌 계측 문제로 분리했다.

## 4. PoC 식별과 재현

재현에 사용한 PoC는 크기와 SHA-256으로 식별했다.

| 파일 | 파일 크기 | Length | 초과 쓰기 | SHA-256 |
|---|---:|---:|---:|---|
| `poc_len17.bin` | 24바이트 | 17 | 1바이트 | `002546b8c963b5dd9236f681dcd52abcace9ad6cb6cd354ffb4ad5128483e541` |
| `poc_len64.bin` | 84바이트 | 64 | 48바이트 | `012a6b742258e0ade6aa6159d5b09fe4549f18653fec1e2da589b09969f00ce0` |

크기는 빠른 식별값이며 SHA-256은 분석한 PoC와 현재 파일의 바이트가 동일한지 확인한다. 해시가 다르면 반드시 변질됐다고 단정할 수는 없지만, 원래 분석한 입력과 다른 파일이므로 동일한 재현으로 취급할 수 없다.

```bash
cd /mnt/d/security-roadmap

wc -c \
  Day101-160/Day117/poc_len17.bin \
  Day101-160/Day117/poc_len64.bin

sha256sum \
  Day101-160/Day117/poc_len17.bin \
  Day101-160/Day117/poc_len64.bin
```

취약한 일반 CLASSIC 빌드와 ASan 빌드에서 다음과 같이 재현했다.

```bash
./Day101-160/Day116/day116_classic \
  < Day101-160/Day117/poc_len17.bin
echo "exit=$?"

ASAN_OPTIONS=abort_on_error=1 \
./Day101-160/Day116/day116_asan \
  < Day101-160/Day117/poc_len17.bin
echo "exit=$?"

./Day101-160/Day116/day116_classic \
  < Day101-160/Day117/poc_len64.bin
echo "exit=$?"
```

관찰 결과는 다음과 같다.

- Length 17은 일반 CLASSIC 빌드에서 `exit 0`으로 정상 종료했지만, ASan은 `memcpy()`의 1바이트 `stack-buffer-overflow`를 탐지했다.
- Length 64는 일반 CLASSIC 빌드에서 3회 모두 SIGSEGV(`exit 139`)가 재현됐다.
- 정상 종료는 관찰 가능한 crash가 없다는 의미일 뿐, 메모리 안전성을 증명하지 않는다. Length 17부터 이미 범위 밖 쓰기가 시작된다.

## 5. Crash 분류와 중복 제거

Crash 파일의 크기와 해시는 입력 식별에 사용했다. Signal과 정규화한 `함수명+offset`은 1차 signature로 사용하되, 최종 분류는 다음 root-cause 증거를 비교했다.

- ASan 오류 종류
- 최초 잘못된 소스 위치
- 손상된 객체
- 위험한 연산
- 누락된 검증

`poc_len17.bin`과 `poc_len64.bin`은 일반 빌드의 종료 결과와 덮어쓴 크기가 다르다. 그러나 두 입력 모두 `parse_record()`의 같은 `memcpy()`가 스택의 `payload[16]`을 초과하고, 같은 목적지 길이 검사가 누락됐다는 공통점이 있다. 따라서 서로 다른 두 취약점이 아니라 같은 stack-buffer-overflow에서 나온 서로 다른 symptom으로 분류했다.

PCGUARD에서 저장된 crash 9개는 다음 이유로 parser 취약점에서 제외했다.

- 1바이트 입력도 parser 검증 전에 죽었다.
- 일반/CLASSIC 및 ASan 재실행에서 parser crash로 재현되지 않았다.
- GDB의 fault 위치가 `parse_record()`가 아니라 AFL coverage bitmap의 `__afl_area_initial` 접근이었다.

따라서 최종 분류 결과는 실제 parser의 길이 검증 누락 1개와 AFL 계측 과정의 false crash 그룹 1개다.

## 6. Root Cause 분석

Length 64 PoC는 전체 84바이트이며, 헤더를 제외한 실제 데이터 77바이트를 포함한다. Length 필드가 64이므로 기존의 source 길이 검사는 통과하지만, 16바이트 목적지에 64바이트를 복사해 48바이트를 초과 기록한다.

GDB에서 확인한 현재 빌드의 스택 배치는 다음과 같다.

- `input_size`: `payload + 24`
- `input`: `payload + 32`
- 복사 범위: `payload + 0`부터 `payload + 63`

62행의 `memcpy()`가 두 변수를 덮은 뒤 `input` 값은 `0x9f9f9f889f9f9f9f`로 손상됐다. 이후 64행이 `input[5]`를 읽으면서 잘못된 주소 `0x9f9f9f889f9f9fa4`를 역참조해 SIGSEGV가 발생했다.

분석 결과를 세 단계로 구분했다.

| 구분 | 내용 |
|---|---|
| Root cause | `length`를 `sizeof(payload)`와 비교하지 않은 검증 누락 |
| 최초 잘못된 연산 | 62행의 범위 밖 `memcpy()` |
| Crash symptom | 64행에서 손상된 `input[5]` 역참조로 발생한 SIGSEGV |

64행의 후속 읽기만 제거하면 눈에 보이는 SIGSEGV는 사라질 수 있지만 62행의 범위 밖 쓰기는 남는다. 따라서 crash 지점을 수정하는 것과 root cause를 수정하는 것은 다르다. 위 스택 offset과 손상된 포인터의 절대값은 현재 컴파일 결과에서 얻은 동적 증거이며, 다른 빌드에서도 동일하다고 일반화하지 않는다.

## 7. 패치와 회귀 테스트

수정본은 위험한 `memcpy()`보다 먼저 목적지 크기를 검사한다.

```c
if (length > sizeof(payload)) {
    fprintf(stderr, "reject at state 4: oversized payload\n");
    return 1;
}

memcpy(payload, input + 7, length);
```

패치 검증에는 정상 입력, 정확한 경계, 경계값+1, 기존 crash PoC를 사용했다.

| Length | 결과 | 시험 목적 |
|---:|---|---|
| 4 | 정상 처리, `exit 0` | 기존의 작은 정상 입력이 계속 동작하는지 확인 |
| 16 | 정상 처리, `exit 0` | 최대 허용 경계값이 잘못 차단되지 않는지 확인 |
| 17 | 복사 전 거부, `exit 1` | 경계값보다 1 큰 첫 잘못된 입력이 거부되는지 확인 |
| 64 | 복사 전 거부, `exit 1` | 기존 crash PoC가 위험한 `memcpy()`에 도달하지 않는지 확인 |

수정본을 ASan으로 실행했을 때 위 네 입력에서 stack-buffer-overflow가 다시 관찰되지 않았다. 이를 근거로 테스트한 경로의 `payload[16]` BOF와 기존 PoC는 차단됐다고 판단했다.

현재 수정본에는 메모리 안전성과 별개의 정리 항목이 남아 있다. 검사가 실행되는 시점에는 enum state가 이미 `ST_PAYLOAD`로 변경됐지만 오류 문자열은 `state 4`를 하드코딩한다. 또한 바로 아래에는 원본의 “의도적 취약점” 주석이 남아 있다. 이 두 항목은 BOF를 다시 만들지는 않지만 로그 정확성과 코드 유지보수를 위해 별도로 수정해야 한다.

## 8. 결론과 분석 한계

이번 실습에서는 toy parser의 입력 형식과 harness를 정의하고, AFL++로 coverage-guided fuzzing을 수행한 뒤 crash를 단독 재현했다. ASan과 GDB를 이용해 crash symptom에서 최초 잘못된 연산까지 역추적하고, 중복 crash와 계측 false crash를 분리했다. 최종 root cause는 공격자가 제어하는 Length를 목적지 `payload[16]`의 크기와 비교하지 않은 검증 누락이었다.

복사 전 목적지 길이 검사와 경계값 회귀 테스트를 통해 기존 Length 17·64 PoC가 위험한 `memcpy()`에 도달하지 않음을 확인했다. 따라서 테스트한 경로의 stack-buffer-overflow가 차단됐다고 말할 수 있다.

다만 다음 한계가 있다.

- 네 개의 대표 입력을 검증한 결과이며 parser 전체의 모든 메모리 취약점 제거를 증명하지 않는다.
- 패치된 빌드에 대해 장시간 fuzzing campaign을 다시 수행한 것은 아니다.
- 120초, 하나의 corpus와 하나의 실행 환경에서 얻은 결과이므로 미발견 경로가 존재할 수 있다.
- PCGUARD 계측 false crash의 원인은 target 취약점과 분리했지만 계측 환경 자체의 근본 문제는 해결하지 않았다.
- GDB에서 확인한 스택 배치와 손상 포인터 값은 현재 컴파일 결과에 종속된다.
- Harness가 실제 caller의 모든 precondition을 보존하는지는 실제 application 통합 환경에서 별도로 검증해야 한다.

## Fuzzing Write-up 필수 항목

재현 가능한 fuzzing 보고서에는 최소한 다음 항목이 필요하다.

- target과 정확한 버전·commit·hash
- 입력 형식과 공격자가 제어하는 필드
- harness의 입력 방식과 실제 caller precondition
- compiler, sanitizer, instrumentation, fuzzer version
- seed corpus와 fuzzing 명령
- 실행 시간, 실행 횟수, coverage/corpus, stability, crash/hang 통계
- PoC 크기·hash와 정확한 재현 명령
- 일반 빌드, sanitizer, debugger의 교차검증
- crash dedup 기준과 false crash 제외 근거
- root cause, 최초 잘못된 연산, crash symptom의 구분
- patch diff와 정상·경계·실패·기존 PoC 회귀 테스트
- 확인하지 못한 범위와 분석 한계
