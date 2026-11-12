# Day145 — GDB로 인자·분기·반환·메모리 권한 확인
실제 수행일: 2026-11-04 / W04 4/5

## 대상과 방법
사용자 로컬 WSL의 GDB/pwndbg로 Day143의 contains_zero_O0를 직접 조작했다. 소스는 [contains_zero.c](../Day143/contains_zero.c), 드라이버는 [contains_zero_test.c](../Day143/contains_zero_test.c). 교사는 명령과 해석을 안내했다. 아래 근거는 사용자가 채팅에 붙인 출력의 발췌이며 완전한 GDB 세션 로그는 아니다. GDB/GCC 버전 및 바이너리 해시는 이번 출력으로 확인하지 않았다.

## 확인한 결과
- 함수 진입에서 rdi는 배열 시작 주소, rsi는 n=3임을 사용자 확인.
- O0 스택 변수: [rbp-0x18]=data, [rbp-0x20]=n, [rbp-0x8]=i.
- 바이트 읽기 직전 rax=data+i. i=1에서 rax-rdi=1, [rax]=00을 실제 확인했다.
- {41,00,42}, n=3: test al,al 뒤 jne가 점프하지 않고 +44의 mov eax,1에 도달했다. 조기 반환 경로상 data[2]를 읽지 않는다.
- 같은 입력을 재실행하여 +71의 pop rbp 직전 eax=1을 확인했다.
- 다음 별도 입력 {41,42,43}, n=3에서는 같은 반환 직전 eax=0. ret 뒤 main+261에서도 eax=0이었다.
- n=0 조건으로 재실행한 확인에서 jb 다음 +66의 mov eax,0에 도달했다. i<n이 거짓이라 원소 읽기 본문을 건너뛴다. 해당 확인의 i/n 메모리 출력은 채팅에 제출되지 않았으므로 첨부 근거를 실제 PC 출력과 구분한다.

## 스택 복원과 CS
반환값 1인 호출의 pop rbp 직전 rsp=rbp=0x7fffffffdf20.
[rsp]=0x7fffffffdf90(main의 저장된 rbp), [rsp+8]=0x55555555522d(복귀 주소).
사용자는 pop 후 rbp=df90/rsp=df28을 예상했고, ret의 rip=[rsp], rsp+=8을 설명했다. 최종 실제 출력은 rip=0x55555555522d(main+123), rsp=0x7fffffffdf30, eax=1이었다. pop 직후 별도 레지스터 출력은 제출되지 않았다.
main 복귀 후 [rbp-0x4c] 같은 위치는 복원된 main의 프레임을 기준으로 해석한다.

info proc mappings에서 코드 0x555555555000~0x555555556000은 r-xp, 스택 0x7ffffffde000~0x7ffffffff000은 rw-p.
ret은 스택의 주소값을 읽은 뒤 그 주소의 코드에서 실행을 이어간다. 저장 위치의 읽기 권한과 목적지의 실행 권한을 구분했다. 사용자는 스택 주소로 실행을 옮기면 현재 권한에서 실행 불가라고 답했다. 실제 NX fault를 발생시키는 실험은 하지 않았다. 이 자료는 런타임 매핑 확인이며 새 readelf 섹션/세그먼트 대조 실험은 아니다.

## 오답·교정·복습
- test al,al 뒤 jne를 두 피연산자의 같고 다름으로 혼동했다. test는 AND 결과, cmp는 뺄셈 결과로 ZF를 설정하며 jne는 ZF=0에서 점프함을 설명받았다. al=42의 test에서 처음 오답, 조건을 cmp al,al로 바꾼 질문에서는 점프하지 않는다고 교정했다. test 조건의 다음 독립 회상은 추후 재확인한다.
- 함수 입구에서 읽은 eax=-8315를 반환값으로 해석하지 않았다. 중단점은 n=3에서 두 번 걸렸고, 다음 호출의 입력은 41 42 43이었다. 교사가 호출을 명확히 구분하지 않은 점을 수정했다. 중간 finish의 전체 출력이 없어 정확한 조작 순서는 단정하지 않는다.
- Day140/141의 ABI·메모리 경계 원리를 Day143 대상으로 적용했다. data[3]={41,42,43}에 n=4를 전달하면 i=3도 허용되어 범위 밖 읽기를 막지 못한다고 사용자가 설명했다. n은 실제 읽을 수 있는 범위 이하여야 한다. 이 반례는 코드 추론이며 새 ASan 실행이 아니다.

## 한계와 다음
사용자가 직접 도구를 조작했지만 명령은 교사 안내를 받았으며 독립 도구 숙달 판정은 아니다. 성능·새 ASan·전체 레지스터 보존·call 직전 상태까지 새로 측정한 것으로 기록하지 않는다. 시간은 측정하지 않았다.
다음 Day146 / W04 5/5: 기존 7바이트 파서의 경계·용량 검사와 실제 ASan 근거 재검증, ELF 섹션/세그먼트 조건 변경 정리. test/jne 회상은 해당 분석에 연결한다. 최소 발표 시간은 완료 요건으로 삼지 않는다.


## 2026-11-12 추가 보완 — ELF `.bss`/ `.data` 조건 변경 실측 및 반론 검증
**대상:** Day145 CS의 누락된 섹션·세그먼트 조건 변경 실험. 이 보완은 2026-11-04 원래 실습 이후에 추가된 별도 증거다.

두 빌드는 동일 소스를 사용하고 `INIT` 매크로만 전환했다. 사용자가 로컬에서 `gcc -O0 -g -fno-common -DINIT=0 elf_layout_probe.c -o elf_bss`, `-DINIT=1 ... -o elf_data`로 컴파일하고 `readelf -sW/-SW/-lW` 결과를 채팅에 제출했다. 사용자 로컬 `elf_layout_probe.c`의 원본 파일은 아직 Git에 업로드되지 않았으므로 아래는 교사가 제시했던 실험 설계의 참고 코드이며 사용자 파일의 완전한 바이트 동일성을 보장하지 않는다:

```c
#include <stdio.h>
#if INIT
unsigned char probe[16] = {0x41};
#else
unsigned char probe[16];
#endif
int main(void) { printf("%02x\n", probe[0]); return 0; }
```

**예상:** 사용자는 `INIT=0`이 `.bss`, `INIT=1`이 `.data`라고 예측했다. 초기에는 `.bss`가 런타임 메모리를 차지하는지 몰랐지만, `FileSiz=0x200, MemSiz=0x210` 설명용 사례에서 추가 `0x10`B가 실행 중 존재하고 0으로 초기화된다고 답했다.

**사용자 제출 심볼 및 섹션 출력:**
```text
$ readelf -sW elf_bss | grep probe
11: ... FILE LOCAL DEFAULT ABS elf_layout_probe.c
21: 0000000000004020 16 OBJECT GLOBAL DEFAULT 26 probe
$ readelf -sW elf_data | grep probe
11: ... FILE LOCAL DEFAULT ABS elf_layout_probe.c
21: 0000000000004010 16 OBJECT GLOBAL DEFAULT 25 probe

$ readelf -SW elf_bss | grep -E '\.(data|bss)'
[25] .data PROGBITS 0000000000004000 003000 000010 00 WA 0 0 8
[26] .bss  NOBITS   0000000000004010 003010 000020 00 WA 0 0 16
$ readelf -SW elf_data | grep -E '\.(data|bss)'
[25] .data PROGBITS 0000000000004000 003000 000020 00 WA 0 0 16
[26] .bss  NOBITS   0000000000004020 003020 000008 00 WA 0 0 1
```

사용자 제출 마지막 RW `LOAD`:
```text
elf_bss:  LOAD 0x002db8 0x0000000000003db8 0x0000000000003db8 0x000258 0x000278 RW 0x1000
elf_data: LOAD 0x002db8 0x0000000000003db8 0x0000000000003db8 0x000268 0x000270 RW 0x1000
```

- `elf_bss`: `MemSiz-FileSiz=0x278-0x258=0x20`(32B).
- `elf_data`: `MemSiz-FileSiz=0x270-0x268=0x08`(8B).
- 사용자가 먼저 `elf_bss`의 차이가 더 크다고 판단했다. 두 바이너리에서 `probe`는 16B로 동일하나 다른 섹션에 들어감이 심볼 `Ndx` 26/25로 확인된다.
- 마지막 반론: "NOBITS이므로 .bss는 실행 중 메모리에도 존재하지 않는다"라는 주장에 대해 사용자가 **"파일에는 저장되지 않지만 실행 중 메모리에는 존재함"**이라고 반박했다. 예상→실측→반론의 필수 순서를 충족.
- 제한: `probe`를 직접 출력한 `./elf_bss`/`./elf_data` 결과는 이번 발췌에 없음. `Section to Segment mapping` 전체 행도 미제출이나, 마지막 RW `LOAD`의 파일/메모리 증가 차이를 실제 출력으로 확인했다. 이 결과는 파일/메모리 크기 및 ELF 섹션 배치 근거이며, 실시간 매핑을 별도로 재확인했다고 주장하지 않는다. 소스·실행파일 로컬 생성은 사용자 보고, Git 첨부는 현재 작성한 설명 텍스트다.

**판정:** Day145 CS 보완 **완료**. 해당 누락 항목을 닫고 Day146 독립 설명 보완으로 이동. 기존 2026-11-04 Day145 자료와 역사적 한계는 유지한다.
