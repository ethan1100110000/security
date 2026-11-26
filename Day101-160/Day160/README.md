# FINAL Day160 — W07 4/5: RELRO / PIE / NX 통제 조건 변경

**Status: NOT STARTED.** 최종 엑셀 `01_일일124` Day160: "RELRO·PIE·NX 등 3개 보호 설정을 바꾸어 가능한 경로/불가능 경로 분류하고 근거 제출". CS는 FULL/PARTIAL RELRO와 GOT 쓰기 반례. 이전 기록 Day043·050 ROP 실패/정렬 복습. 당일 D+2 복습: Day158(2026-11-24 완료) leak 신뢰도.

이 교육용 GNU/Linux x86-64 실습은 **동일한 C 소스의 보호 옵션만 바꿔** 원인과 결과를 비교한다. Day161의 블라인드 신규 바이너리 문제와 구분한다. `lab160.c`는 공개된 실습 소스이며, 실행 성공·보호 수준을 미리 알려주지는 않는다.

## 0. 준비

```bash
cd /mnt/d/security-roadmap
git status --short
git pull --ff-only
cd Day101-160/Day160
bash build.sh
bash record.sh
```

`record.sh`는 터미널 기록을 `day160_session.log`에 덧붙이고 매번 flush한다. 중간에 끊기면 재실행해 **같은 로그에 이어 기록**. 기록이 빠졌다면 콘솔에 실제 보였던 출력만 별도 출처로 남기고, 절대 가짜 원본 로그를 만들지 않는다.

## 1. 가설 — 실행 전에 예측

기본형 한 개와 **단일 보호 설정 변경** 세 개. 같은 소스에서 원인이 보호 설정인지 검증한다.

| 실험 파일 (`bin/`) | RELRO | PIE | NX | 3회 `main` 주소 변화? | 동일 값 GOT 쓰기 성공? | stack의 RET 1B 실행 성공? |
| --- | --- | --- | --- | --- | --- | --- |
| lab_full_pie_nx | Full | ON | ON | ? | ? | ? |
| lab_partial_pie_nx | Partial | ON | ON | ? | ? | ? |
| lab_full_nopie_nx | Full | OFF | ON | ? | ? | ? |
| lab_full_pie_execstack | Full | ON | OFF | ? | ? | ? |

예측마다 이유 한 줄. ASLR ON 환경을 가정. **Writable GOT = 취약점 존재**가 아니며, **PIE OFF = libc 주소 고정**이 아니다.

## 2. 정적 검증

```bash
file bin/lab_full_pie_nx
checksec --file=bin/lab_full_pie_nx
readelf -hW bin/lab_full_pie_nx
readelf -lW bin/lab_full_pie_nx
readelf -dW bin/lab_full_pie_nx
readelf -rW bin/lab_full_pie_nx
```

동일한 명령을 네 변형에 적용. 비교할 실제 ELF 지표: `ET_DYN / ET_EXEC`, `GNU_STACK RW / RWE`, `GNU_RELRO`, `BIND_NOW` 또는 `FLAGS_1 NOW`, `puts`의 `R_X86_64_JUMP_SLOT`. Partial에도 `GNU_RELRO`가 존재할 수 있으므로 섹션/세그먼트의 존재만으로 Full을 판정하지 않는다.

## 3. 동적 검증

### A. 코드 주소 재배치

`./bin/lab_full_pie_nx addr`를 독립 프로세스 3회 실행. 다른 세 바이너리도 각각 3회. `main`, `marker`, 그리고 두 주소 차이를 기록. 세 번의 표본이 전체 가능한 주소/재사용 확률을 증명하지는 않는다.

### B. NX

`./bin/lab_full_pie_nx stack` 및 다른 세 변형 실행. 프로그램은 자식 프로세스의 스택에 있는 **x86-64 RET(0xC3) 한 바이트만 실행**해 본다. 부모가 자식의 signal/exit 결과를 보고함. 이것은 **스택 페이지 실행 권한 테스트**이며 임의 코드 실행 취약점을 발견했다는 뜻이 아니다.

### C. RELRO / GOT 페이지

`nm`에서 **해당 파일의** `main` 정적 심볼 값, `readelf -rW`에서 **그 파일의** `puts@...` `JUMP_SLOT` 재배치 슬롯 오프셋을 찾는다.

```bash
nm -n bin/lab_full_pie_nx | grep ' main$'
readelf -rW bin/lab_full_pie_nx | grep 'puts@'
./bin/lab_full_pie_nx got 0x<main_sym_value> 0x<puts_JUMP_SLOT_offset>
```

이 작업을 네 파일 각각 수행. 프로그램은 현재 프로세스의 `&main - main_sym_value`로 로드 베이스를 계산한 뒤 GOT 슬롯 주소를 산출하고, **fork한 자식 안에서만 그 슬롯의 기존 값을 읽고 똑같은 값으로 다시 쓰기**를 시도한다. 결과는 메모리 쓰기 권한의 증거이며 GOT overwrite exploit의 증거는 아니다. 잘못된 심볼/슬롯 주소로 실험하면 판정 무효. 가능하면 GOT 슬롯이 속한 프로세스 매핑 권한 `/proc/<pid>/maps` 또는 `vmmap`과 교차 확인. GDB `set`으로 메모리를 바꾸는 테스트는 ptrace 동작이 페이지 권한을 우회할 수 있어 쓰기 가능의 유효한 입증이 아니다.

## 4. 설명·변형 반례·증빙 게이트

- 네 바이너리의 예측 ↔ 실측 결과표, `readelf`와 실제 실행이라는 **근거 두 종류**, 한 보호 설정만 바꿨다는 입증.
- 가능한/불가능한 경로를 분리: 정상 GOT 참조/쓰기 허용, 기존 코드 ROP, 스택 바이트 실행, 주소 재배치, 취약점 존재는 각각 다른 주장.
- 최소 1개 반례: GOT writable이어도 쓰기 취약점이 없다면?, PIE OFF라도 libc ASLR은?, Full RELRO인데 saved RIP overwrite는? 등에서 **새 조건을 고르고 증거/논리로 설명**.
- CS: Full/Partial RELRO와 lazy/now relocation, GOT 슬롯 실제 메모리 보호 속성.
- Day043·050: 이전 Git의 ret 추가/제거 사례에서 시작 RSP 정렬을 바꾼 새로운 소비 슬롯 문제를 무힌트로 계산.
- Day158 D+2: 새로운 leak 유형의 출처/신뢰도/반례를 분류.
- `day160_writeup.md` (예측·명령·버전·결과·반례·도움 여부), 빌드 원본, 로컬 실행 로그, 증거의 출처와 학생 Git 커밋 SHA 확보 후 완료.

**운영:** 메인/CS/복습/Write-up 모두 수행해야 Day 완료. 교사 설명이나 빌드만으로 통과 처리하지 않는다. 관찰과 추정을 반드시 구분한다.
