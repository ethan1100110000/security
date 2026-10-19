# Day137 — O0/O2 동작 비교와 호출 규약

- 학습일: 2026-10-19
- 계획: W03 바이너리 기초 / 3주차 1/5
- 상태: 메인·CS·복습 범위 종료. Day136 종료 후 같은 날 진행.
- 실제 시간: 미측정
- 실행 주체: 교사 컨테이너, 사용자 WSL 결과가 아니다.
- 새 실행: 동일한 복사 함수·테스트를 O0/O2로 빌드해 5사례씩 실행, 출력 diff 및 실제 바이너리 역어셈블.
- 성능 시간 측정·sanitizer·GDB 동적 레지스터 관찰은 수행하지 않았다.

## 1. 메인: 최적화 옵션과 예상

사용자는 O0/O2의 의미를 몰라 설명을 받았다.
- O0는 최적화를 거의 하지 않아 소스와 명령의 대응을 읽기 쉬운 경우가 많다.
- O2는 효율적인 코드를 생성하려고 명령·레지스터·스택 사용을 바꿀 수 있다.
- 정의된 동작에 대해서는 관찰 가능한 프로그램 동작을 보존해야 한다. 명령어 순서와 rbp 프레임까지 동일해야 한다는 뜻은 아니다.

사용자는 최적화 이유만으로 복사 내용·반환값이 바뀌면 안 된다고 답했다. 프레임 명령 생략이 오류인지 질문에는 모르겠다고 답해, rbp 프레임은 한 구현 방식이고 ret은 rsp 기준 복귀 주소를 읽는다는 설명을 받았다.

## 2. 비교 구성

Day134 copy_test.c에서 같은 copy_data 본문을 copy_data.c로 분리하고, 기존 다섯 테스트는 copy_test_driver.c에 선언과 함께 유지했다. 함수와 드라이버는 별도 translation unit이고 LTO는 사용하지 않았다. 테스트 호출이 함수 안으로 인라인되거나 호출 문맥만으로 대체되는 영향을 줄여, 분리된 copy_data 구현을 비교하려는 구성이다.

동일 컴파일러·공통 플래그·소스·사례에서 최적화 옵션만 바꿨다.
```bash
cc -std=c11 -Wall -Wextra -Werror -O0 -g copy_data.c copy_test_driver.c -o copy_test_O0
cc -std=c11 -Wall -Wextra -Werror -O2 -g copy_data.c copy_test_driver.c -o copy_test_O2
./copy_test_O0 > results_O0.txt
./copy_test_O2 > results_O2.txt
diff -u results_O0.txt results_O2.txt
objdump -d -Mintel --disassemble=copy_data copy_test_O0 > disassembly_O0.txt
objdump -d -Mintel --disassemble=copy_data copy_test_O2 > disassembly_O2.txt
```

각 빌드·실행·diff·objdump 종료 코드는 모두 0이다. 개별 실패를 놓치지 않도록 명령별 결과를 확인한다. 재빌드 환경에 따라 명령·주소·파일 해시는 달라질 수 있다.

### 실행 결과
| 사례 | O0 반환 | O2 반환 | 목적지 검증 |
|---|---:|---:|---|
| src_size=4, length=4 | 0 | 0 | 복사 4바이트 일치, 나머지 AA 유지 |
| src_size=4, length=5 | 1 | 1 | 전체 AA 유지 |
| src_size=32, length=16 | 0 | 0 | 16바이트 일치 |
| src_size=32, length=17 | 2 | 2 | 전체 AA 유지 |
| 41 00 42 43, length=4 | 0 | 0 | NUL 포함 내용 일치, 나머지 유지 |

capacity는 모두 16이다. 양쪽 모두 5/5 통과했고 전체 출력 diff도 일치했다. 이 다섯 사례의 동일 동작을 관찰했으며 모든 입력에 대한 동등성을 시험으로 증명한 것은 아니다. 길이 0 실행은 Day136 별도 사례이며 이 다섯 사례에 포함하지 않는다.

## 3. 실제 생성 코드의 차이

| 항목 | O0 | O2 |
|---|---|---|
| rbp 프레임 | push rbp; mov rbp,rsp; sub rsp,0x30 | rbp 프레임 없음 |
| 인자 유지 | rbp 상대 스택 저장 | 주로 레지스터 사용 |
| 원본 길이 분기 | src_size>=length에서 계속 경로로 jae | src_size<length에서 거부 경로로 jb |
| 정상 호출 앞뒤 스택 | 프레임에 포함 | sub rsp,8 / add rsp,8 |
| 종료 | 공통 leave; ret | 정상·거부 경로에 ret |

### O2 첫 검사
```asm
mov r9, rcx
mov ecx, 1
cmp r9, r8
jb  1318
```

r9는 src_size, r8은 length다. 사용자는 if(src_size < length) return 1에 해당한다고 답했다. 실제 거부 경로는 ecx의 준비된 결과를 eax로 옮겨 ret한다. 주소 1318은 이번 바이너리의 정적 표시이고 런타임 고정 절대 주소로 일반화하지 않는다.

O0와 O2의 점프 명령 종류가 달라도 비교 피연산자 순서와 점프 목적지를 함께 읽으면 같은 원본 길이 거부 조건을 구현한 것을 확인할 수 있다.

## 4. CS: 호출 인자 이동과 레지스터 보존

Linux x86-64 System V 호출 규약에서 copy_data 진입 시 rdi=dst, rsi=capacity, rdx=src, rcx=src_size, r8=length다. O2의 memcpy 호출 준비는 다음 이동을 포함했다.

```asm
mov rax, rdx
mov rdx, r8
mov rsi, rax
call memcpy
```

사용자는 호출 직전 rsi=src, rdx=length를 맞혔다. rdi의 dst는 그대로여서 memcpy(dst, src, length)가 된다. 레지스터는 함수 전체에서 한 변수만 나타내는 것이 아니다.

rsi는 이 규약에서 caller-saved이므로 호출 이후 같은 주소값을 보존한다고 가정하지 않는다. 사용자는 보존 가정은 안 될 것이라고 예상했고, 교사가 caller-saved 의미를 설명했다. 실제 memcpy가 이번 실행에서 rsi를 어떻게 바꿨는지는 관찰하지 않았다.

호출 이후 src가 필요하면 호출자 프레임에 확보한 스택 공간에 주소값을 보관한 뒤 읽는 방법이 가능하다. 사용자는 가능하다고 답했다. 호출자가 값을 보존하는 것과 원본 객체 수명 보장은 별개다. 확보하지 않은 공간이나 호출로 보존되지 않는 red zone을 임의의 저장 장소로 가정하지 않는다.

## 5. 복습: 구조 변화와 성능 결론

두 빌드의 다섯 테스트 결과가 같고 O2의 rbp 프레임이 없어졌다는 것은 실제 확인했다. 그러나 실행 시간은 측정하지 않았다.

사용자는 “최적화 버전이 더 가볍고 빠르지 않을까”를 예상하면서도 결론은 못 낸다고 답했다. 교사는 더 빠름은 가설이며 동일 동작과 기계어 구조 차이가 오늘 근거의 범위라고 정리했다. 동일 결과를 “같은 성능”이라고 부르면 동작 정확성과 실행 속도를 혼동할 수 있어 구분한다.

최적화로 항상 빨라진다거나 프레임 생략만으로 속도 향상 크기를 계산할 수는 없다. 실제 성능 비교는 공정한 측정 조건과 반복 관찰이 추가로 필요하며 오늘 필수 과제로 추가하지 않았다.

## 6. 산출물·완료와 다음 학습

- copy_data.c, copy_test_driver.c: 비교에 사용한 실제 소스
- results_O0.txt, results_O2.txt: 전체 실행 출력
- disassembly_O0.txt, disassembly_O2.txt: 정적 역어셈블
- environment.txt, sha256.txt: 환경과 최종 파일 식별
- 바이너리는 해시만 기록하고 재빌드한다.

메인 실제 비교·CS 인자/보존 문답·복습 근거 구분을 종료했다. W03 전체 완료·독립 5분 발표·성능 측정·GDB 단일 단계 수행을 주장하지 않는다. 그림으로 소스와 함수 전체를 연결하는 W03 주간 산출물은 앞으로 작성할 과제다.

다음: Day138 / 3주차 2/5 — 소스의 조건·반환과 최적화된 분기를 대응해 읽기. CS는 호출 규약·레지스터 보존을 작은 코드와 연결하고, 복습은 기존 디버거 기록에서 관찰과 추론을 구분한다.
