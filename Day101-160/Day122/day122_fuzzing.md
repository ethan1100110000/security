# Day122 — TinyXML-2 Target Build with Sanitizer

## 1. 목표

Day121에서 선정한 TinyXML-2를 고정 커밋으로 가져와 다음 빌드가 가능한지 검증한다.

* 일반 Clang 빌드
* ASan·UBSan 빌드
* AFL++ 계측 빌드
* 공식 테스트 실행
* 빌드 실패 및 비결정적 실행 원인 분리

이번 단계는 타겟의 buildability와 sanitizer 실행환경을 검증하는 단계다. 실제 fuzz harness 작성과 fuzzing campaign 실행은 아직 포함하지 않는다.

---

## 2. Target identity

* Repository: `https://github.com/leethomason/tinyxml2`
* Commit:

```text
8224e427b655b83dae5e2298f1e6919523a78737
```

다음 명령으로 대상 커밋을 확인했다.

```bash
git rev-parse HEAD
git status --short --branch
```

결과:

```text
8224e427b655b83dae5e2298f1e6919523a78737
## HEAD (no branch)
```

`HEAD (no branch)`는 특정 커밋을 detached HEAD 상태로 checkout했기 때문에 나타난 정상적인 결과다.

주요 환경:

```text
clang++: Ubuntu clang version 14.0.0-1ubuntu1.1
afl-clang-fast++ backend: Ubuntu clang version 13.0.1-2ubuntu2.2
AFL++: 4.00c
CMake: environment.txt 참조
```

---

## 3. 일반 Clang 빌드

다음과 같이 일반 Debug 빌드를 생성했다.

```bash
cmake -S . -B build-plain \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_SHARED_LIBS=OFF \
  -Dtinyxml2_BUILD_TESTING=ON

cmake --build build-plain --parallel
ctest --test-dir build-plain --output-on-failure
```

공식 테스트 결과:

```text
100% tests passed, 0 tests failed out of 1
Total Test time (real) = 0.39 sec
```

생성된 실행 파일은 다음과 같았다.

```text
ELF 64-bit LSB pie executable
x86-64
dynamically linked
with debug_info
not stripped
```

생성물:

```text
build-plain/libtinyxml2.a
build-plain/xmltest
```

일반 빌드의 `xmltest`를 20회 반복했을 때 모두 정상 종료했다.

이 결과로 고정한 TinyXML-2 소스가 현재 환경에서 일반 Clang 빌드와 공식 테스트 실행이 가능함을 확인했다.

---

## 4. 최초 ASan·UBSan 빌드

처음에는 PIE를 유지한 상태에서 ASan과 UBSan을 함께 적용했다.

```bash
cmake -S . -B build-sanitize \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DBUILD_SHARED_LIBS=OFF \
  -Dtinyxml2_BUILD_TESTING=ON \
  -DCMAKE_CXX_FLAGS="-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=undefined" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
```

`compile_commands.json`에서 다음 플래그가 실제 적용된 것을 확인했다.

```text
-fsanitize=address,undefined
```

그러나 공식 테스트와 직접 실행에서 간헐적인 SIGSEGV가 발생했다.

관찰된 결과:

* 성공한 실행에서는 모든 공식 테스트 통과
* 실패한 실행에서는 `exit=139`
* CTest는 실패 시 `exit=8` 반환
* 실패 로그 크기: 0바이트
* ASan·UBSan 오류 보고서 없음
* 오류 종류, 최초 잘못된 연산, 함수 및 소스 행을 확인할 수 없음

`exit=139`는 `128 + SIGSEGV(11)`이므로 프로세스가 SIGSEGV로 종료됐다는 뜻이다. CTest의 `exit=8`은 자식 테스트의 원시 종료 코드가 아니라 하나 이상의 테스트가 실패했다는 CTest의 종료 코드다.

정상적인 ASan 탐지라면 오류 종류, 접근 크기, stack trace, 함수와 소스 행 및 `SUMMARY`가 포함된 보고서가 있어야 한다. 이번 실패에는 해당 증거가 없었으므로 TinyXML-2 취약점으로 분류하지 않았다.

---

## 5. LeakSanitizer 영향 분리

LeakSanitizer의 종료 단계 검사가 원인인지 확인하기 위해 `detect_leaks` 설정을 나눠 반복했다.

### Leak 검사 비활성화

```text
noleak run=3 raw_exit=139
noleak run=7 raw_exit=139
```

### Leak 검사 활성화

```text
leak run=5 raw_exit=139
leak run=8 raw_exit=139
```

Leak 검사 활성화 여부와 관계없이 각각 10회 중 2회 SIGSEGV가 발생했다.

따라서 `detect_leaks` 설정만으로 설명되는 문제는 아니라고 판단했다.

---

## 6. ASan과 UBSan 분리

ASan만 적용한 별도 빌드를 생성했다.

```bash
cmake -S . -B build-asan \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_SHARED_LIBS=OFF \
  -Dtinyxml2_BUILD_TESTING=ON \
  -DCMAKE_CXX_FLAGS="-O1 -g -fsanitize=address -fno-omit-frame-pointer" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"
```

ASan 단독 빌드에서도 간헐적인 `exit=139`가 발생했다.

따라서 UBSan이 없어도 문제가 재현되며, 실패 조건을 ASan 측으로 좁혔다. 다만 ASan이 TinyXML-2의 오류를 정상적으로 탐지한 보고서는 여전히 생성되지 않았다.

---

## 7. 최소 ASan probe 대조 실험

TinyXML-2와 무관한 정상 프로그램을 작성해 같은 ASan 조건으로 컴파일했다.

```cpp
#include <cstdio>

int main() {
    char* buffer = new char[16];
    buffer[0] = 'A';
    std::printf("%c\n", buffer[0]);
    delete[] buffer;
    return 0;
}
```

이 프로그램에는 의도적인 범위 밖 접근이나 해제 후 사용이 없다.

PIE 상태에서 ASan을 적용한 최소 probe에서도 간헐적인 `exit=139`가 발생했다. 따라서 TinyXML-2에서만 발생하는 문제로 볼 수 없었다.

---

## 8. 파일시스템 영향 분리

`/mnt/d`에서 빌드한 ASan probe를 WSL의 `/tmp`로 복사해 20회 실행했다.

실패 회차:

```text
tmp run=3 raw_exit=139
tmp run=5 raw_exit=139
tmp run=8 raw_exit=139
tmp run=12 raw_exit=139
tmp run=16 raw_exit=139
tmp run=17 raw_exit=139
tmp run=18 raw_exit=139
tmp run=20 raw_exit=139
```

`/tmp`에서도 20회 중 8회 SIGSEGV가 발생했다.

따라서 `/mnt/d` 마운트나 Windows 파일시스템만의 문제는 아니라고 판단했다.

---

## 9. PIE와 non-PIE 비교

최소 ASan probe를 non-PIE로 다시 빌드했다.

```bash
clang++ -O1 -g \
  -fsanitize=address \
  -fno-omit-frame-pointer \
  -fno-pie -no-pie \
  asan_probe.cpp -o asan_probe_nopie
```

non-PIE ASan probe는 20회 반복 실행에서 모두 정상 종료했다.

비교 결과:

| 빌드                   | 위치       | 반복 결과        |
| -------------------- | -------- | ------------ |
| PIE + ASan probe     | `/mnt/d` | 간헐적 SIGSEGV  |
| PIE + ASan probe     | `/tmp`   | 8/20 SIGSEGV |
| non-PIE + ASan probe | `/mnt/d` | 20/20 정상     |
| 일반 TinyXML-2         | `/mnt/d` | 20/20 정상     |

이 결과로 현재 WSL 환경에서 PIE·ASLR 메모리 배치와 ASan runtime의 상호작용이 간헐적 SIGSEGV와 관련 있음을 확인했다.

다만 ASan 내부의 정확한 충돌 위치를 보여주는 보고서가 없으므로 구체적인 내부 root cause까지 확정하지는 않았다.

---

## 10. 안정적인 ASan·UBSan 빌드

시스템 전체 ASLR을 비활성화하지 않고, 검사 전용 TinyXML-2 바이너리에만 non-PIE를 적용했다.

```bash
cmake -S . -B build-sanitize-nopie \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DBUILD_SHARED_LIBS=OFF \
  -Dtinyxml2_BUILD_TESTING=ON \
  -DCMAKE_POSITION_INDEPENDENT_CODE=OFF \
  -DCMAKE_CXX_FLAGS="-O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=undefined -fno-pie" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined -no-pie"
```

생성된 `xmltest`가 PIE 실행 파일이 아니라 일반 ELF 실행 파일임을 `file`로 확인했다.

다음 환경으로 공식 테스트를 10회 반복했다.

```bash
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
ctest --test-dir build-sanitize-nopie --output-on-failure
```

결과:

```text
10/10 정상 종료
ASan 오류 보고서 없음
UBSan runtime error 없음
CTest 실패 없음
```

PIE를 끈 것은 TinyXML-2의 취약점을 수정한 것이 아니다. 현재 WSL에서 sanitizer 실행환경을 안정화하기 위한 검사 전용 설정이다.

또한 non-PIE는 PIE 방어 기능을 제거하므로 배포용 빌드 설정으로 일반화하지 않는다.

---

## 11. ASan과 UBSan의 역할

### ASan

AddressSanitizer는 다음과 같은 메모리 접근 및 할당 오류를 탐지한다.

* stack buffer overflow
* heap buffer overflow
* use-after-free
* double free
* 잘못된 메모리 읽기·쓰기

Day117에서 일반 빌드가 정상 종료했던 Length 17 입력의 1바이트 stack-buffer-overflow를 탐지한 도구가 ASan이다.

일반 빌드의 `exit=0`은 메모리 안전을 의미하지 않는다. 잘못된 메모리 접근이 발생해도 현재 메모리 배치에서 즉시 crash로 이어지지 않을 수 있기 때문이다.

### UBSan

UndefinedBehaviorSanitizer는 C/C++의 undefined behavior를 탐지한다.

예시:

* 잘못된 shift
* signed integer overflow
* misaligned access
* 잘못된 형 변환
* 일부 잘못된 포인터 및 타입 사용

ASan과 UBSan은 탐지 대상이 다르므로 함께 사용하면 메모리 오류와 undefined behavior를 서로 보완해 검사할 수 있다.

---

## 12. AFL++ 계측 빌드

AFL++ 계측 빌드는 별도 디렉터리에서 생성했다.

```bash
cmake -S . -B build-afl \
  -DCMAKE_CXX_COMPILER=afl-clang-fast++ \
  -DCMAKE_BUILD_TYPE=Debug \
  -DBUILD_SHARED_LIBS=OFF \
  -Dtinyxml2_BUILD_TESTING=ON

cmake --build build-afl --parallel
ctest --test-dir build-afl --output-on-failure
```

AFL++ 빌드에서도 공식 테스트가 통과했다.

계측 여부는 다음 명령으로 확인했다.

```bash
afl-showmap -q \
  -o xmltest_afl.map \
  -- ./build-afl/xmltest
```

결과:

```text
2197 xmltest_afl.map
```

`xmltest` 1회 실행에서 AFL coverage map에 2,197개 tuple이 기록됐다.

이 값은 다음을 의미하지 않는다.

* 취약점 2,197개
* 소스 분기문 2,197개
* 전체 attack surface의 크기
* fuzzing corpus 2,197개

이는 현재 AFL++ 계측 바이너리의 해당 실행에서 관찰한 coverage tuple 수다.

또한 `xmltest`는 AFL++의 변이 입력을 직접 받아 `XMLDocument::Parse(data, size)`에 전달하는 fuzz harness가 아니다. 따라서 이번 결과는 AFL++ 계측 빌드가 가능하다는 증거이며, 실제 fuzzing 준비 완료를 뜻하지 않는다.

---

## 13. 결론

고정한 TinyXML-2 커밋에서 다음을 확인했다.

* 일반 Clang 빌드 성공
* 일반 공식 테스트 통과
* ASan·UBSan 계측 빌드 성공
* non-PIE 적용 후 sanitizer 공식 테스트 10회 안정적 통과
* AFL++ 계측 빌드 성공
* AFL++ 공식 테스트 통과
* `afl-showmap`에서 2,197개 tuple 기록
* 최소 대조 프로그램을 이용해 TinyXML-2 문제와 실행환경 문제를 분리

최초 PIE sanitizer 빌드에서 발생한 간헐적 SIGSEGV는 정상적인 sanitizer 오류 보고서를 생성하지 않았다. 동일 현상이 TinyXML-2와 무관한 최소 ASan probe 및 `/tmp`에서도 재현됐고, non-PIE probe에서는 사라졌다.

따라서 이번 SIGSEGV를 TinyXML-2 취약점으로 분류하지 않고, 현재 WSL의 PIE·ASLR 배치와 ASan runtime의 상호작용 문제로 기록한다. 구체적인 ASan 내부 root cause는 보고서가 없어 확정하지 않는다.

아직 다음 작업은 수행하지 않았다.

* 변이 XML을 메모리로 전달하는 fuzz harness 작성
* seed corpus 구성
* AFL++ fuzzing campaign 실행
* crash 수집 및 재현
* TinyXML-2의 취약점 유무 판단

따라서 이번 Day122에서 확인한 범위는 일반·sanitizer·AFL++ 빌드 가능성과 공식 테스트 실행까지다.
