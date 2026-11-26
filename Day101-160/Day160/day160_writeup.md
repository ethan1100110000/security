# Day160 — FINAL W07 4/5 — 보호기법 통제 실험 (진행 중)

- 학습 시작: 2026-11-26 (KST)
- 상태: 초기 가설 제출, 실측·CS·복습·학생 로그 미완료.
- 실험 설계: 동일 `lab160.c`에서 기본 Full RELRO/PIE ON/NX ON과 각각 한 설정만 바꾼 Partial RELRO, PIE OFF, NX OFF 변형을 비교. 세부 과제는 [README](README.md)를 따른다.

## 1. 실행 전 학생의 초기 가설 — 채팅 원문

질문: ① Full→Partial RELRO에서 GOT 쓰기 권한, ② PIE OFF + ASLR ON에서 `main` 주소 변화, ③ NX OFF에서 스택 코드 실행과 ROP 필요성.

학생 답변 원문: `1. write 가능 2. 안바뀜 3. 실행은 가능 rop필요할 수 있음`.

- 잠정 평가: 세 방향 모두 타당하나 ①에서 모든 GOT 슬롯이 writable인 것은 아니며 일반적으로 lazy binding을 사용하는 `.got.plt`와 RELRO 보호된 `.got`를 구별해야 함. ②는 일반적인 Linux non-PIE main 코드에 대한 예측으로, libc/stack ASLR 비활성화를 뜻하지 않음. ③은 스택 실행 권한이라는 한 조건만 바뀐 것이며, 임의 분기 제어나 제약 조건을 해결했다고 뜻하지 않음.
- **아직 빌드·readelf·GOT 쓰기·주소 반복·스택 명령 실행의 학생 측 실측 출력 미제출.** 예상 결과를 측정 결과처럼 기록하지 않는다.

## 2. 첫 번째 학생 측 정적 관찰 (2026-11-26)

학생은 기본형 `bin/lab_full_pie_nx`에 대하여 `checksec`의 보호기법이 모두 예상대로 나왔고, `GNU_STACK`이 `RW`이며 실행 권한이 없다고 **요약 보고**했다 (checksec/프로그램 헤더 전체 원문은 아직 제출 안 됨).

학생이 직접 제출한 `readelf -dW bin/lab_full_pie_nx | grep -E 'BIND_NOW|FLAGS'` 출력:

```text
0x000000000000001e (FLAGS)              BIND_NOW
0x000000006ffffffb (FLAGS_1)            Flags: NOW PIE
```

`DT_FLAGS`의 `BIND_NOW` 및 `DT_FLAGS_1`의 `NOW`는 함수 재배치의 즉시 해결을 뜻하고, `PIE` 비트는 PIE 실행 파일이라는 동적 플래그다. 이는 Full RELRO 동작과 일관되지만 **해당 두 줄만으로 GOT 페이지의 읽기 전용 매핑을 직접 관찰한 것은 아님**. 다음으로 `GNU_RELRO` 세그먼트 존재와 Partial 비교 및 동적 GOT 자기값 쓰기를 확인한다.

## 3. Partial RELRO와 puts@plt 관찰 — 학생 원문

학생이 제출한 `readelf -dW bin/lab_partial_pie_nx | grep -E 'BIND_NOW|FLAGS'`:

```text
0x000000006ffffffb (FLAGS_1)            Flags: PIE
```

따라서 Full 변형에서 나타났던 `BIND_NOW`/`NOW`가 없고, 즉시 바인딩 요구가 관찰되지 않음.

학생이 제출한 `objdump -d -M intel bin/lab_partial_pie_nx | grep -A 5 '<puts@plt>:'`:

```asm
0000000000001140 <puts@plt>:
    1140: f3 0f 1e fa           endbr64
    1144: f2 ff 25 dd 2e 00 00 bnd jmp QWORD PTR [rip+0x2edd] # 4028 <puts@GLIBC_2.2.5>
    114b: 0f 1f 44 00 00     nop DWORD PTR [rax+rax*1+0x0]
```

`0x4028`은 `puts`의 GOT 슬롯 **ELF 이미지 상대 주소**로, 이 명령은 런타임 해당 슬롯 안의 8바이트 포인터로 간접 점프한다. 이 PLT 엔트리에는 직접 `push`가 보이지 않으므로 예전 Day157의 전형적인 `puts@plt+6` 형태를 이 바이너리에 기계적으로 대입할 수 없음. 별도 `.plt` 해석 스텁과 `R_X86_64_JUMP_SLOT` 및 실제 첫 호출 전 GOT 값을 추적할 예정; 아직 런타임 GOT 값·메모리 권한은 관측하지 않았음.

## 4. 다음 작업

사용자는 `bash build.sh; bash record.sh` 이후 정적 ELF 특성으로 각 파일의 설정을 확인하고, 먼저 기본형의 `file`/`checksec`와 `readelf` 결과를 수집한다. 이후 4개 조건의 3회 주소 관측, 동일값 GOT 슬롯 쓰기, 스택에서 1바이트 RET 실행을 비교하고 사전 가설과 대조한다. 이전 Day043/050 정렬, Day158 D+2 주소 누출 신뢰도, CS Full/Partial RELRO 복습 후 원본 로그·커밋을 확인한다.
