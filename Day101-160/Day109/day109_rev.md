# Day109 - Stripped Binary Analysis

## 분석 대상

* 바이너리: `Day101-160/Day105/day105_stripped_exam`
* SHA-256: `05985db435131d96ba48a4715ce0f583d8f3c2d1761b24a4a77de036d9bffbd1`
* 형식: stripped x86-64 PIE ELF
* 보호 기법: Full RELRO, Canary, NX, PIE
* 추가 속성: SHSTK, IBT

## 재현 명령

```bash
# 저장소 루트에서 실행
cd Day101-160/Day105

sha256sum -c SHA256SUMS
checksec --file=day105_stripped_exam

objdump -d -M intel \
  --start-address=0x1476 \
  --stop-address=0x148c \
  day105_stripped_exam
```

## 함수 흐름

main은 `_start`에서 `__libc_start_main`의 첫 번째 인자로 전달되는 주소를 추적해 찾았다.

* `inspect`: `fgets`로 입력받고 개행을 제거한 뒤 `printf("%.24s", ...)`로 최대 24바이트를 출력한다.
* `submit`: `[rbp-0x40]`의 stack buffer에 `read`로 최대 `0xa0`바이트를 입력받고 Receipt 값을 계산한다.
* `quit`: `Bye`를 출력하고 종료한다.

## BOF 증거

Ghidra와 `objdump`에서 다음 명령을 동일하게 확인했다.

```asm
lea  rax,[rbp-0x40]
mov  edx,0xa0
mov  rsi,rax
mov  edi,0
call read@plt
```

`read()`의 인자는 다음과 같다.

* `RDI = 0`: 표준 입력
* `RSI = rbp-0x40`: stack buffer
* `RDX = 0xa0`: 최대 입력 길이 160바이트

버퍼 시작점과 stack 항목의 거리를 계산하면 Canary offset은 `0x38`, saved RBP는 `0x40`, saved RIP는 `0x48`이다. 따라서 160바이트 입력으로 Canary, saved RBP와 saved RIP를 덮을 수 있는 BOF가 존재한다.

Receipt 계산은 입력 앞부분의 최대 24바이트만 사용하므로 offset `0x38`의 Canary에는 도달하지 않는다. 따라서 Receipt 출력은 Canary leak으로 사용할 수 없다.

## 실패 케이스

```bash
python3 -c 'import sys; sys.stdout.buffer.write(b"submit\n" + b"A"*56)' \
  | ./day105_stripped_exam

python3 -c 'import sys; sys.stdout.buffer.write(b"submit\n" + b"A"*57)' \
  | ./day105_stripped_exam
```

56바이트 입력은 Canary를 덮지 않아 `Receipt: 2d01ec38` 출력까지 진행됐다. 실제 `read()` 반환값은 56이며, 이후 Receipt 계산에 사용하는 길이만 최대 `0x18`로 제한된다. 이미 수행된 메모리 쓰기는 취소되지 않으므로 이 제한은 BOF 자체에 영향을 주지 않는다.

57바이트 입력은 offset `0x38`에 있는 Canary의 첫 바이트를 덮어 `stack smashing detected`와 함께 종료됐다. 이를 통해 정적 분석으로 계산한 Canary offset `0x38`을 동적으로 검증했다.

## Decoy 함수

전체 문자열과 XREF를 조사한 결과, 두 개의 24바이트 영역을 0으로 초기화하고 `memcmp(..., 0x18)`로 비교하는 함수를 확인했다.

이 함수에는 incoming call/data XREF가 없고 main의 도달 가능한 호출 흐름에도 포함되지 않는다. 또한 두 비교 대상이 모두 0이므로 강제로 호출하더라도 비교 결과는 0이며 실패 출력 조건이 성립하지 않는다. 따라서 정상 실행 행위가 아닌 decoy 함수로 판단했다.

## 결론

BOF를 이용해 saved RIP까지 덮을 수 있지만, 함수 반환 전 Canary 검사를 통과하려면 원본 Canary 값이 필요하다. 또한 PIE base와 libc base를 계산할 leak primitive도 확인되지 않았다.

따라서 BOF는 존재하지만 현재 확인한 기능만으로는 안정적인 제어 흐름 탈취나 ret2libc exploit을 구성하기 어렵다.
