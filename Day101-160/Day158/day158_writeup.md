# Day158 — FINAL W07 2/5: leak 신뢰도·PIE/libc Base·CS·D+7 복습 (최종 기록)

> 상태: **2026-11-24 Day158 W07 2/5 완료**. 아래 기록은 학생이 채팅으로 직접 제출한 숫자/설명과 교사 질문·교정을 구분한다. Git에 이 교사 작성 문서가 존재한다는 것만으로 Day158 완료로 처리하지 않는다.

## 1. 실습물/환경
- 교사가 제공한 stripped PIE 로컬 ELF: `day158_probe.elf`. 최초 파일은 `GLIBC_2.38 not found`로 사용자 WSL에서 실행 실패. 교사가 glibc 2.34 이하 의존성으로 다시 빌드하여 `unpack.sh` 복원 후 학생 실행.
- 명령 1~5, 0 종료; 학생 최초 제출:
```text
audit> 1   observation=0x5722e5943189
audit> 2   observation=0e10
audit> 3   observation=read the evidence, not the label
audit> 4   observation=0x7fa2cb680e10
audit> 5   observation=0x7ffff7abcdef
```
- 독립 실행 3회 후 학생 관찰: 2번과 4번의 마지막 4자리 항상 동일. 1번은 상위 부분 변동·하위 부분 고정. 5번은 매 실행 동일. 4번의 변화는 명시된 3회 원본 출력 전부는 아직 미제출. 주소는 **실행/프로세스별로 다르므로 서로 다른 세션의 값을 한 계산에 섞지 않음.**

## 2. 신뢰도 판단: 독립 검증
- 1번: GDB에서 `0x555555555000~0x555555556000 r-xp`, ELF 파일 오프셋 `0x1000` 매핑 확인. 이에 따라 `PIE base=0x555555554000`. 같은 GDB 세션에서 1번 누출 `0x555555555189` → `ELF 상대오프셋=0x1189`. `x/8i $rebase(0x1189)` 실측:
```asm
0x555555555189: push rbp
0x55555555518a: mov rbp,rsp
0x55555555518d: sub rsp,0x20
0x555555555191: mov DWORD PTR [rbp-0x14],edi
0x555555555194: mov rax,QWORD PTR fs:0x28
```
  학생은 함수 프롤로그라고 추론, 교사는 독립 함수 시작의 강한 근거이되 프롤로그만으로 절대 확증은 아니라고 설명.
- 2번: 하위 16비트 `0e10`만으로는 전체 주소나 libc base를 하나로 정할 수 없다. 학생은 초기엔 알려진 심볼 오프셋과 GDB base를 결합해볼 수 있다고 생각했다가, **GDB base는 추가 정보**이고 상위 비트 부족 및 소속 함수 불명이 주요 문제라고 교정받음.
- 3번: 문자열이며 주소가 아님.
- 4번: 학생 `info symbol` 확인 결과 libc `puts`. 학생 `readelf -Ws /lib/x86_64-linux-gnu/libc.so.6`에서 `puts@@GLIBC_2.2.5` 심볼 `0x80e10` 직접 관찰. 같은 GDB 세션에서 puts 주소 `0x7ffff7c80e10`을 사용한 계산 `libc base=0x7ffff7c00000`. 학생의 vmmap 관찰은 puts가 `r-xp` 매핑 안이라는 보고; 오프셋 0 매핑 시작과 base 일치한 전문은 아직 직접 제시되지 않음.
- 5번: 학생 GDB 증거:
```text
x/gx 0x7ffff7abcdef → Cannot access memory
vmmap 0x7ffff7abcdef → There are no mappings for specified address or module.
x/8bx 0x7ffff7abcdef → Cannot access memory
```
  이 실행에서는 유효한 맵핑 포인터가 아니므로 포인터 leak으로 신뢰할 수 없으며, 값의 코드 상수 여부는 정적 검증 전.

## 3. Base 계산 오류의 독립 반례 3개
- A: 하위 16비트 누출만 있으면 대상이 `puts`임을 안다고 해도 상위 주소를 복원할 수 없고 가능한 base가 여러 개. 최초 답변에서 상위 비트 부족을 놓쳤으나 교사 교정 후 구분.
- B: 미매핑 `0x7ffff7abcdef`를 libc 주소로 가정하면 잘못된 base 및 연쇄적인 가상 함수주소가 만들어짐. 형식상 16진수라고 참 포인터가 아님.
- C: 동일 세션 `0x555555555189`에서 실제 오프셋 `0x1189` 대신 `0x1289`를 빼면 `base=0x555555553f00`으로 실제 `0x555555554000`보다 `0x100` 낮음. 학생은 4KiB 페이지 정렬에 맞지 않는 점을 직접 오류 근거로 제시. 페이지 정렬은 필요조건이지 충분조건이 아님.

## 4. CS: RIP-relative 주소 및 signed disp32
- 교사 제시 `0x5555555551a0: lea rax,[rip-0x37]`, 길이 7B. 학생이 RIP 기준을 처음에 몰랐고 `[rip-0x37]`을 메모리 읽기로 혼동한 지점 교정.
- 학생이 다음 명령 `0x5555555551a7`을 구하고 `RAX=0x555555555170` 직접 계산. `lea`는 유효주소 자체, `mov`는 해당 주소에 저장된 데이터 읽기임을 구분.
- `disp32=0xffffffc9`, bit31=1이고 signed로 해석하면 `-0x37`. 학생은 signed 해석과 최상위 비트 설명 정답. 실제 ELF의 다른 `lea`를 별도로 동적 검증한 기록은 없음.

## 5. 보안 복습: Day043·050 ROP stack alignment
- 조건: `pop rdi; ret` 진입 `RSP%16=8`. 학생 첫 답: `pop` 후 0, `ret` 후 8, 정상 함수 진입 8 정답. 다만 `ret`를 하나 추가해야 한다고 **최초 오답**.
- 교사 교정: 이미 `system` 진입 8이라 추가 `ret`가 오히려 0으로 바꿔 정렬을 손상할 수 있음. 기존 Day043은 추가, Day050은 제거했던 이유가 당시 도달시점 정렬 차이임.
- 원본 비교: Day154에서 유사 `pop rdi; ret` 조건 정답 기록도 있으나 이번 교정이 필요했음. 독립적인 완전 무힌트 숙련으로 과대판정하지 않음.

## 6. 장기 복습: Day153·154 D+7
- 과거 Day153 ASan 재구성 `stack-buffer-overflow`; Day154 canary/NX 2x2 및 구조체 내부 오염 회상. 합친 새 조건:
```c
struct record { char name[16]; uint64_t marker; };
struct record r = { .marker=0x1122334455667788 };
memcpy(r.name,input,20); // input = 'A' 20개
```
- 학생 최초 답: `marker=0x1122334441414141` 정답. Canary를 건드리지 않으며 데이터 쓰기는 NX의 실행권한 검사가 아니라고 정확하게 설명.
- ASan이 놓칠 수도 있다고 맞게 답했지만 처음에 '쓰기 가능한 메모리라서'라고 이유를 잘못 설명. 교사는 writable과 ASan의 redzone/객체 경계는 별개라고 교정.
- 추가 A: `char buf[16]` 단독 객체에 20B 복사 vs B: 구조체의 `buf[16]` 넘어 `marker` 4B 침범. 학생이 **객체 경계 vs 내부 멤버 경계**를 정확히 설명. 보통 ASan shadow/redzone이 전체 구조체를 보호하나 멤버 사이의 경계는 모두 표시하지 않아 B를 놓칠 수 있다. D+7 개념 인출 및 교정 후 전이 통과. 단 실제 ASan 코드를 새로 실행해 계측 비교한 것은 아님.

## 7. 현재 판정과 남은 증거
- 메인: 다중 누출, 정적/동적 대조, 실패 가정 반례 3개 구술 확인.
- CS: 최종 결과 이해·계산 통과(초기 교사 힌트 필요).
- 보안 복습: ROP 정렬 `ret` 추가 여부 최초 오답 교정.
- D+7: Day153·154 ASan/Canary/NX 구조체 경계 인출 통과(ASan 이유 최초 오류 교정).
- **사용자 원본 3회 실행/최소 vmmap 증거와 libc symbol 출력, 재현 명령 및 사용자 Git commit/push가 아직 확인되지 않았음. Day158 전체 완료 보류.**


## 8. 2026-11-24 — 학생 Git 원본 로그 확인 및 최종 완료 판정
- 학생 커밋: `9bf64d869c7406cb9fbbdeba8be90d0e971200f5` (`Day158: add leak analysis evidence`); `day158_evidence.log` blob `fe70fb3b6a00d77eac1322654480e2054a2a75fe`. `checksec`, `readelf -Ws` puts `0x80e10`, `readelf -lW` 4개의 LOAD, 독립 프로세스 3회 출력이 포함됨.
- 원본 3회 출력: 1번 `0x5c5471ae4189`, `0x56b1ed750189`, `0x55942ca06189` (하위 `0x1189` 일정); 2번 `0e10` 고정; 4번 `0x7edb91080e10`, `0x76645bc80e10`, `0x72a11be80e10` (하위 16비트 `0e10` 동일); 5번 `0x7ffff7abcdef` 고정. 3번 문자열 고정. 이로써 사용자 비교 관찰이 원본 실험 로그로 검증됨.
- **증거 수준 한계**: GDB의 `vmmap`, `x/i`, 주소 매핑 실패 메시지는 학생 채팅 제출 실측이며 이번 Git 로그 자체에는 재현되지 않음. 교사 작성 분석 문서의 GDB 기록을 사용자 Git의 원본 GDB 전체 로그로 주장하지 않는다. 단 본 Day의 검증 질의에 답하고 핵심 계산 및 반례 설명을 수행했으며 사용자의 정적/동적 증거 일부가 대화에 제출되어 있다.
- ROP 정렬의 추가 ret 최초 오답과 ASan writable 근거 최초 오답, RIP 계산 초기 힌트를 지우지 않음. 수정·재평가 후 이해 도달로 기록.
- **최종 판정: Day158 / FINAL W07 2/5 완료. 다음 Day159 / W07 3/5 미시작.** 이전 섹션 7의 '확인 대기'는 당시 상태를 의미하며 이 섹션의 최신 판정을 따른다.
