# Day153 / FINAL W06 1/4 — first-look stripped ELF (진행 중)

## 대상
- `target_elf.gz.b64`: **교사가 생성한 로컬 교육용 x86-64 Linux stripped ELF**를 gzip+base64로 보관한 것. 소스는 첫 독립 분석 전에는 공개하지 않음.
- 본 과제는 사용자 소유의 격리된 로컬 바이너리 학습용이며 타인 서비스/호스트 대상이 아님.
- Original ELF SHA-256: `901617de462bd1701f851c30a931be83fdd7e712e43f7da5e7b009ec85db811b`
- 교사 원본 C SHA-256 (최초 블라인드 분석 보존용): `7dc7d5ac5f9c0bc5f06033cb5df601a9284fa8531ff7c9616845163fc5859104` (추후 공개 및 검증 예정).
- **메인·CS·복습·재현·Git 원본 증거가 끝나기 전 완료 판정 금지.**

## 실행 준비 — Linux / WSL
```bash
cd /mnt/d/security-roadmap
git pull --ff-only
cd Day101-160/Day153
base64 -d target_elf.gz.b64 | gzip -dc > target_elf
chmod +x target_elf
sha256sum target_elf
file target_elf
```

## 첫 번째 독립 단계 (소스/해설을 보지 않기)
```bash
checksec --file=./target_elf  # 없으면: pwn checksec ./target_elf
strings -tx target_elf | head -60
readelf -W -h target_elf
objdump -d -M intel target_elf | less
```
1. 사용자가 `main` 후보와 입력의 도달 경로를 식별하고 주소·근거를 제출한다.
2. 입력 길이가 어디에서 만들어지고 검사되는지, 검사가 실제 복사 대상의 경계와 일치하는지 **독립 가설**을 제출한다.
3. 이후 GDB에서 레지스터·스택·복사 길이 실측을 하고, **동일 조건 ASan 비교 빌드**를 교사가 제공한다.
4. 정상·오류·경계 반례를 명령/입력으로 재현한 뒤 정확한 위치 차이를 설명한다.
5. SysV AMD64 call/ret·7~9번째 인자 추적, Day040~050 과거 Git 스택/ROP 회상→원본 대조→무힌트 반례 및 Write-up/commit 수행.

## 증거 계약
- 학생의 **실제로 실행한 명령·입력 파일·관찰 로그·환경**과 추론을 구분해 기록한다.
- 분석을 시작한 뒤 소스가 공개되더라도 최초 무힌트 분석 판단과 AI 도움 범위를 보존한다.
- `target_elf`는 위 텍스트에 포함된 원본 바이너리의 로컬 생성물이다. Git에는 복원이 가능한 `*.b64`를 보존한다.
- 패치 소스·ASan 비교 자료·replay.sh·사용자의 실제 실행 로그/Write-up은 후속 분석 후 추가한다. 현재는 **실험 준비 단계**.

## First independent triage (user reported, not yet verified)
- 사용자 첫 답변(원본 도구 출력 미제출): 보호 기법은 NX 활성과 Partial RELRO를 관찰했다고 보고함. **Canary/PIE 상태는 미제출**. checksec 원문은 확인되지 않아 최종 판정 아님.
- 입력 함수는 `read`라고 보고함. ELF import 목록에 보이는 것인지 실제 입력 처리 경로에서 `read@plt` 호출을 확인한 것인지는 아직 검증되지 않음.
- `main` 탐색 독립 전략: 처음 호출의 첫 인자에 main이 들어있을 거라고 추정. **교정 필요**: 첫 번째 call이라는 순서는 보편적인 식별 기준이 아니며, ELF entry에서 `_start` 초기화 코드가 `__libc_start_main`에 넘기는 첫 인자(RDI)를 근거로 추적해야 함. 입력 도달 경로는 XREF/콜사이트/호출 인자 확인으로 검증 예정.
- 다음 제출 요구: (1) `checksec` 네 보호기법 전체 결과, (2) `readelf -W -h` entry 및 `__libc_start_main` 호출 전후 역어셈블, (3) 실제 `read` call site와 그 앞에서 설정되는 RDI/RSI/RDX를 보여주는 어셈블리.
- **현시점 검증 등급:** 가설 단계. 사용자 GDB 동적 계측·ASan 대조·취약점 재현 아직 없음.

## Independent static evidence: _start → main candidate

- 사용자 독립 제출: stripped ELF .text `0x401090`부터 시작하는 코드. `xor ebp,ebp`, `mov r9,rdx`, `pop rsi`, `mov rdx,rsp`, stack alignment, `mov rdi,0x4012e9` at `0x4010a4`, `call QWORD PTR [rip+0x2f27]` at `0x4010ab`, `hlt` at `0x4010b1`.
- 사용자의 **최초 독립 가설**: `0x4012e9`가 main. 이 근거는 통상적인 x86-64 glibc `_start`에서 `__libc_start_main`의 첫 인자(`rdi`)로 main 함수 포인터를 넘기는 패턴과 일치하므로 **강한 main 후보**임.
- **확증되지 않은 부분:** `call QWORD PTR [rip+0x2f27]`의 간접 호출 대상은 아직 실제 GOT dynamic relocation을 사용자가 보여주지 않음. `0x403fd8`이 `__libc_start_main` relocation인지 확인해야 심볼 문자열이 없는 상태에서도 startup 경로가 확증됨. objdump의 근처 임의 라벨 `<memcpy@plt+0x2f58>`는 호출 목표의 심볼을 보장하지 않음.
- 사용자 요청으로 분석 절차·명령어 선제 지시를 중단하고 독립 분석 결과만 증거 관점에서 평가. 사용자는 Ghidra가 판단을 더 쉽게 만든다고 언급; Ghidra 추정도 GOT relocation/교차참조가 뒷받침할 때 강해짐. **메인 취약점 분석/실행 실측은 아직 미제출.**

## Independent static analysis: input-to-helper control flow
- 사용자 자체 역어셈블 분석(원본 전체 디스어셈블/동적 출력은 미제출): 추정 main에서 puts 후 `read(0,rbp-0xd0,0xc0)`; 반환값이 양수일 때 helper `0x401176`을 호출하고 그렇지 않으면 종료. helper의 인자로 입력 시작 주소 및 읽은 바이트 수를 RDI/RSI로 전달한다고 파악함.
- 사용자: `0x401176` 내부 `sub rsp,0x60`을 지역 스택 공간 확보로 해석. `[rbp-0x58]`에 입력 포인터, `[rbp-0x60]`에 읽은 길이를 저장한다고 해석. 길이가 7을 넘는 조건 분기 뒤 4바이트 `memcmp` 비교와 추가 연산/출력 경로를 식별함.
- 교사 확인할 해석 경계: `read` 반환값 0은 EOF, -1은 오류이므로 양수가 아닌 경로를 전부 'read 실패'로 부르면 부정확. `[rbp-offset]` 저장 슬롯과 해당 포인터가 가리키는 버퍼는 별개. `read(...,0xc0)`의 길이만으로 BOF를 확정할 수 없으며 실제 버퍼 경계와 helper의 길이 사용을 더 확인해야 함.
- **학습자 독립 진도**: entry→main→입력→길이 판정→helper→4바이트 비교까지 흐름 요약. '조건에 맞으면 뭔가 출력'은 가설로 표시하며 구체적인 검증/복사 지점/메모리 안전성은 미확정.
- 사용자 선호에 따라 특정 명령어 실행을 선제 지시하지 않고 **입력 길이가 어디에서 무엇을 제한하는지** 한 가지 핵심 증거에 집중하는 방식으로 평가.


## Independent evidence: parsing two little-endian fields

- 사용자 직접 제출한 0x4011de~0x40122d 역어셈블: `[rbp-0x58]`의 포인터로부터 입력 `+4`, `+5`, `+6`, `+7` 위치의 1바이트 값을 읽고 `shl eax,8` / `or eax,edx`를 수행, 결과를 `[rbp-0x12]` 및 `[rbp-0x14]`에 WORD로 저장함. `cmp WORD PTR [rbp-0x12],0x70; ja 0x401240`가 뒤따름.
- **관찰된 구조:** 앞 두 바이트 `input[4]` (low) + `input[5]<<8` (high)가 한 16비트 무부호 값, 다음 두 바이트 `input[6]` + `input[7]<<8`이 또 다른 16비트 값. 사실상 little-endian 16-bit 필드 두 개를 수동 복원하는 코드 패턴. 입력 데이터를 덮어쓰는 것이 아니라 스택 로컬 WORD에 저장.
- `ja`는 unsigned 초과. 첫 WORD가 `0x70` (112)보다 큰 경우 0x401240 분기. 그 이후 다른 WORD의 유효성 검사, 복사 길이/목적지 버퍼 등은 **해당 excerpt만으로 확정 불가**.
- 독립 관찰: 사용자는 이전 `[rbp-0x58]`가 입력 자체라기보다 주소일 가능성을 자발적으로 재평가하고 명령 구간 제출. 해석은 교사 검증과 힌트가 섞인 단계로 분리 기록. 아직 동적 실행·크래시·취약점 확증 없음.


## Independent flow: local byte-sum loop observed
- 사용자 직접 제출한 `0x40128a~0x4012e8` 역어셈블과 해석 질문. 앞서 `[rbp-0x58]` 입력 버퍼 포인터를 로드해 바이트별로 접근한다는 사실은 파악했으나, `movzx`와 반복문 의미에서 막힘.
- 교사의 근거 설명: `movzx eax,BYTE PTR [rax]`와 `movzx eax,al`은 읽은 1바이트를 32비트로 zero-extend (후자는 연속 사용 시 중복). `[rbp-0x4]`은 32비트 누적 합, `[rbp-0x10]`은 64비트 반복 인덱스. `lea rdx,[rbp-0x50]; add rax,rdx`는 로컬 영역 시작 + 인덱스 주소를 만들고 1바이트 읽어 합산함. 루프는 `idx < value1` AND `idx <= 0x2f` 범위에서 수행해 최대 48바이트를 합산. 마지막 `printf`는 `rsi=value2`, `rdx=sum`의 데이터를 넘김; 실제 형식 문자열은 미제출.
- **핵심 검증 분리:** 여기서 확인된 48바이트 제한은 **읽고 합산하는 루프의 제한**이며, 앞선 별도 복사 작업에도 같은 제한이 적용된다는 증거가 아님. `[rbp-0x50]`는 로컬 데이터 영역 시작 후보로, 정확한 버퍼 크기·복사 크기·목적지·복사 호출 여부는 앞선 명령 구간에서 확인 필요. 이 단계에서 취약점 성공을 주장하지 않음.
- 사용자 독립 관찰은 스택 슬롯 저장, 입력의 리틀엔디언 16비트 필드 두 개, 후속 로컬 영역 바이트 합산 루프 발견까지. 구체적 루프 의미는 교사 설명을 받았으므로 후속 반례에서 독립 검증 필요.


## Independent evidence: memcpy call site

사용자가 독립적으로 `0x40126f~0x401285` 코드에서 `memcpy@plt` 호출을 찾아 제출했다.
```asm
40126f: movzx edx,WORD PTR [rbp-0x12]
401273: mov   rax,QWORD PTR [rbp-0x58]
401277: lea   rcx,[rax+0x8]
40127b: lea   rax,[rbp-0x50]
40127f: mov   rsi,rcx
401282: mov   rdi,rax
401285: call  401080 <memcpy@plt>
```
- 관찰에 근거한 인자 매핑: `memcpy(dst,src,n)`의 `rdi=&local([rbp-0x50])`, `rsi=input_ptr+8`, `rdx=(uint16_t)WORD PTR [rbp-0x12]`(앞서 헤더 `input[4..5]`에서 복원한 첫 값). **목적지는 스택 프레임 내부 로컬 영역, 원본은 8바이트 헤더 이후**.
- 복사의 크기는 이전 합산 루프의 `i<=0x2f` 제한과 무관함. 앞선 `value1<=0x70` 검사에 통과할 수 있으므로 복사 크기와 실제 목적지 유효 용량 비교가 필요. 현재 제시된 코드만으로 `local`의 정식 배열 선언 크기와 해당 경로의 기타 검사가 모두 확정되진 않음.
- 독립 실행/크래시 증거 아직 없음. 다음 교육 목표: 사용자가 `rdi,rsi,rdx`의 의미와 목적지 크기보다 복사 크기가 커질 수 있는지를 자신의 말로 판단.


## Boundary reasoning: memcpy length versus destination size

- 교사는 목적지 용량이 **48바이트라는 가정**을 제시하고 `value1=80`일 때 경계 초과 크기와 기존 `value1<=112` 검사 의미를 질문.
- 사용자 독립 답변: 초과 바이트는 **32바이트**이고, **value1 검사를 목적지 버퍼 크기와 비교해야 한다**고 답함. 가정하에서는 두 답 모두 정확 (`80-48=32`).
- 이 시점에서 **48바이트는 과제에서 가정한 크기**이며 실제 ELF의 로컬 목적지 배열 선언 크기는 아직 소스/디스어셈블로 확증되지 않았다. 정확한 문턱과 실측 BOF 여부는 독립 확인 예정.
- 추적할 별도 조건: 목적지 용량 검사(`n<=capacity`)와 입력에서 실제 확보한 소스 바이트 수 검사(`read_return>=8+n`)를 혼동하면 안 됨. 현재 제출된 검사 중 `read_return>7`만으로 `input+8`에서 `n`바이트를 유효 입력 범위 내에 확보했다고 보장할 수 없음.
- 다음 작업은 교사가 명령어를 지정하지 않고 사용자가 자신의 방법으로 실제 목적지 용량과 경계 전후 메모리 효과/크래시를 확인. **Day153 전체 미완료**.

## memcmp 4-byte identifier observed in GDB
- 사용자가 GDB에서 비교 대상 `0x402011`의 4바이트 값을 `50 58 4e 31` (hex)로 직접 보고함. ASCII로 **`PXN1`**. 이는 `memcmp`의 4바이트 비교가 성공하려면 필요한 입력 식별값으로 해석됨.
- 해당 값은 사용자의 **직접 관찰**이며 `memcmp` 결과/조건 통과, `memcpy`에서의 RDI/RSI/RDX 값 및 실제 취약점 재현은 **아직 검증되지 않음**.


## GDB evidence: memcpy arguments for 32-byte benign probe
- 사용자 로컬 GDB/Pwndbg에서 입력 파일 `probe.bin` (`PXN1`, 첫 16-bit 값=32, 둘째 값=0, `A` 32개)과 `break *0x401285`로 `memcpy` 직전 레지스터 출력 **실측**:
```
rdi 0x7fffffffde60
rsi 0x7fffffffdec8
rdx 0x20
```
- SysV AMD64 `memcpy(dst, src, n)` 기준: 목적지 `0x7fffffffde60`(스택 로컬 영역), 원본 `0x7fffffffdec8`(원본 입력의 헤더 뒤), 크기 `0x20=32`바이트. 정적 분석한 `rbp-0x50`, `input+8`, `value1` 매핑과 일치.
- 이 실측은 **`memcpy` 호출 직전 인자 검증**이며, 실제 복사 완료 또는 버퍼 경계 초과를 입증하는 증거는 아직 아님. 다음은 목적지 주소를 GDB convenience 변수로 보존하고 `ni` 전후 메모리를 비교, 이후 경계 값으로 별도 검증한다.

## GDB evidence: 32-byte memcpy destination changed to 0x41
- 앞서 `probe.bin` (PXN1 + 16비트 길이32 + 16비트 보조값0 + `A` 32B)으로 `memcpy` 직전 `RDI=0x7fffffffde60`, `RSI=0x7fffffffdec8`, `RDX=0x20`을 사용자 직접 GDB 관찰함.
- 사용자 후속 구두 보고: `ni` 후 목적지에 **`0x41`이 32바이트 기록되었다**. 실제 GDB 복사 전/후 메모리 덤프 전문은 아직 공유되지 않았으므로 보고한 범위만 실측으로 인정.
- 이는 `memcpy` 32B 정상 동작을 뒷받침하며 **메모리 경계 초과 또는 정확한 목적지 배열 크기 48B를 단독으로 확증하지는 않음**. 다음 단계는 길이 48/49 등 변경 조건에서 소스 크기와 목적지 주변 변화/경계 증거 비교.


## Ghidra-assisted control-flow reconstruction (student independent)

- 사용자 독립 작업: Ghidra 디컴파일과 기존 어셈블리를 직접 대조해 전체 처리 흐름을 재구성. `main`은 `puts` → `read(0, local_input, 0xc0)` → `read` 결과 비양수에서 "no input" 출력, 양수에서 helper 호출. 메인 버퍼는 Ghidra에서 약 200바이트로 보이나 실제 선언 크기는 불명. 사용자 이번 메시지에서 메인 `read`의 스택 주소를 `rbp-0x58`이라고 표현했으나 **앞선 정적 관찰의 `main: rbp-0xd0`와 상충**. 별도 함수에서 `[rbp-0x58]`에 입력 포인터를 저장한 것과 혼동했는지 확인 필요. 각 함수의 RBP는 다름.
- helper 처리 흐름 사용자 설명: `read_count < 8`이면 "short record" 반환 -1, 선두 네 바이트 `PXN1`과 `memcmp` 실패 시 "bad marker" 반환 -2. 통과 시 4~5바이트에서 little-endian 16비트 `a`, 6~7바이트에서 `b` 복원. `a < 113` 및 `a <= read_count-8`가 통과하면 `memset(local,0,0x30)` 후 `memcpy(local,input+8,a)`. 이후 `i<a && i<0x30`의 누적합 `sum+=local[i]`; `printf("accepted: mode=%u sum=%u",b,sum)`, 반환 0.
- **새롭게 확인한 중요한 안전성 검사:** `a<=read_count-8`로 입력 쪽에 헤더 이후 `a` 바이트가 존재하는지 제한한다는 사용자 decompile 결과. 이전 메모의 "read_count>7만으로 복사 원본이 확보되지 않음"은 **이 후속 검사로 보완됨**. 다만 이번에는 Ghidra 기반 사용자 정적 해석이며 정확한 조건의 분기와 source-byte bounds는 바이너리/GDB 대조 대상.
- **추측 금지:** `read` 최대 요청 길이 `0xc0=192`는 메인 로컬 배열 크기를 의미하지 않음. `memset(...,0x30)`는 초기화 바이트 수 48이고 실제 `local` 배열 크기의 확증은 아님. Ghidra가 약 60바이트로 추정한 것과 충돌하며 **정확한 경계가 미해결**. 최대 허용 복사 길이 112는 확인된 조건에 부합하고, 실제 목적지 용량에 대한 외부 검증 없이는 오버플로우 정확한 시작점/영향을 확정하지 않음.
- **독립성 평가:** 사용자 스스로 구조·조건·값 해석·합산 루프·출력 의미를 순서대로 완결해 설명. Ghidra 보조 활용 사실과 이전 교사 힌트(e.g. memcpy 인자, little-endian 해석)는 보존. 현재는 메인 **정적 흐름 복원 완료**, 실기 메인 목표의 **실제 경계/ASan 검증은 미완료**. 사용자는 필요시 명령어를 요청하겠다고 한 선호를 존중하고 교사는 우선 결과와 논리 평가에 집중.


## GDB 80-byte probe: bytes beyond 48-byte memset region overwritten

- 사용자 입력 `probe80.bin` (헤더 PXN1, a=80, b=0, 본문 'A'×80), `memcpy` 직후 목적지 시작 `0x7fffffffde60` 기준 `dest+48 = 0x7fffffffde90`의 32B 원본 메모리 덤프:
```
0x7fffffffde90: 00 00 00 00 00 00 00 00
0x7fffffffde98: 90 df ff ff 00 00 50 00
0x7fffffffdea0: 00 00 00 00 00 00 00 00
0x7fffffffdea8: e0 e2 ff f7 ff 7f 00 00
```
- 같은 32B가 `ni`로 복사를 실행한 뒤 전부 `41`로 변경된 것을 사용자 직접 덤프로 입증. `0x41='A'`. `memset(...,0x30)`으로 초기화한 영역 뒤 32B를 입력이 제어하는 실측 증거.
- 디스어셈블리상 `dest=[rbp-0x50]`이므로 이 실행에서 `rbp=0x7fffffffdeb0`으로 유도됨. `a=[rbp-0x12]=0x7fffffffde9e` (복사 전 메모리 바이트 `50 00`=80), `b=[rbp-0x14]=0x7fffffffde9c` (복사 전 `00 00`=0). 둘 다 `dest+48..+79` 영역에 위치해 `41 41`로 덮임. 단 **RBP 실측/디스어셈블리 연결이 전제**; 다음 과제는 실제 `a`/`b` 값을 GDB로 확인하고 `continue` 후 출력 영향 관찰.
- 결론 구분: 실제 스택 메모리 오염 및 a/b 로컬 슬롯 덮어쓰기 정황 매우 강함. 정확한 **원본 소스 수준 배열 크기**는 여전히 48B memset만으로 확증되지 않았으며, 아직 ASan과 런타임 출력 대조가 남음. 메인 Day153 미완료.


## Runtime confirmation: corrupted a/b local slots alter accepted output

- 사용자가 `probe80.bin` (PXN1, a=80, b=0, 'A'×80) 실험에서 `memcpy` 직후 GDB `x/2hx $rbp-0x14` 출력 `0x7fffffffde9c: 0x4141 0x4141`을 직접 제출. 따라서 `b=[rbp-0x14]`와 `a=[rbp-0x12]` 두 16비트 로컬 슬롯이 모두 입력의 `AA`로 오염된 사실 확인.
- `continue` 결과 **`accepted: mode=16705 sum=3120`**. `0x4141=16705`이므로 mode 입력이 0이었는데 로컬 `b` 덮어쓰기로 출력값이 바뀐 것과 일치. `3120=48*0x41`은 합산 루프가 최대 48바이트의 'A'를 더한 결과와 일치.
- 정적 제어흐름 + 실측 32/80B + 변수오염 + 사용자 가시 출력 변화를 연결하는 **실질적 스택 메모리 오염 재현** 성공. `memset` 48B는 배열의 형식상 선언 크기를 증명하지 않으므로 원본 소스 기준 정확한 경계 확정 및 ASan 경고/미경고 비교는 별도 과제.
- 후속 학습 방향: 이 실행의 `dest=[rbp-0x50]` 기준 80B 복사는 `rbp-1`까지 쓰고 종료하므로 저장된 RBP/복귀 주소는 **이 페이로드로는** 덮이지 않음. 다음은 사용자가 저장 프레임/복귀주소까지의 오프셋을 독립적으로 추론하고 가장 작은 검증 입력을 선택. 입력/목적지 범위 겹침 여부도 큰 크기에서는 주의. 전체 Day153은 CS·복습·ASan·원본 증거 기록 전까지 미완료.
