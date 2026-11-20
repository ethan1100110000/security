# Day156 W06 독립 재평가 — 2026-11-20

새로 제작한 로컬 교육용 ELF. SHA256: `565d297112871b10645becfa1527b7a589af7392a99cfb2ea8c1793b461430cc`. 교사 환경에서 실행 및 정상/경계 입력 동작 확인. 풀이와 소스는 첫 독립 답변 제출 전 비공개.

## 시작
`bash unpack.sh`로 복원 및 해시 검증. `./day156_retest.elf`를 분석한다.

## 제출 기준
- 입력의 제어 범위, 검증 흐름, 메모리 경계 및 오류 원인, 실제 보호 효과와 한계, 수정안.
- 정상 입력과 반례를 직접 설계. 사전 예상과 실제 출력 및 코드/메모리 근거 2종 이상, 실행 환경과 재현 입력/명령 저장.
- 첫 독립 분석 답안을 제출한 다음 교사가 평가한다. 교사 도움을 받으면 그 사실을 기록한다. 수정 후 검증은 첫 답안 평가 뒤 별도로 진행한다.
- 실제 로그와 재현 입력, 최종 보고서 및 Git 커밋 확인 전 통과하지 않는다. 이전 CS/복습 교정은 반복하지 않음.

상태: 시작. 학생 결과 미제출.

## 최초 무힌트 정적 분석 — 학생 답변
- 학생은 도움 없이 fgets, 개행 제거, strlen, 헤더 문자 범위 및 구분자, 본문 최대 길이, 초기화, category 저장, memcpy, 별도의 종료 문자 쓰기와 출력 흐름을 복원했다. 특히 최대 본문 복사 자체와 뒤따르는 null 종료 문자 쓰기를 구분하고, 최대 길이에서 인접 category가 변경되어 출력이 바뀔 가능성을 제시했다.
- 교사는 배포된 압축 ELF를 복원하여 SHA256 일치 및 objdump로 학생 분석을 대조했다. 핵심 원인 추론은 정확하다. 보완: 헤더를 포함한 문자열 길이는 2보다 커야 하므로 본문 허용 길이는 1~24이며, 원본 C 배열 선언 크기는 어셈블리 배치만으로 무조건 확정하지 않는다.
- 판정: 독립 정적 분석 통과. 학생의 실제 정상/경계/거부 입력 출력, 종료 상태, 메모리 관찰, 보호기법 확인 및 수정 설계는 아직 미제출. 교사 측 정적 검증을 학생 측 동적 실측으로 대체하지 않는다.

## 독립 동적 검증 — 학생 제출 원문
- 학생이 직접 실행한 터미널 출력과 pwndbg 관찰 및 checksec 결과를 채팅으로 제출. 아래는 제출 내용에서 프롬프트와 불필요한 공백을 정리한 발췌이며 교사의 재실행 결과가 아니다.

```text
$ ./day156_retest.elf
Enter a record:
1:aaaaaaaaaa
category=1 size=10 label=aaaaaaaaaa
$ ./day156_retest.elf
Enter a record:
2:bbbbbaaaa
category=2 size=9 label=bbbbbaaaa
$ ./day156_retest.elf
Enter a record:
3:aaaaaaaaaaaaaaaaaaaaaaaa
category=0 size=24 label=aaaaaaaaaaaaaaaaaaaaaaaa
```

```text
0x555555555271    mov    byte ptr [rax], 0               [0x7fffffffdee8] <= 0
pwndbg> x/gx $rbp - 0x18
0x7fffffffdee8: 0x0000000000000002
pwndbg> x/gx $rbp - 0x18
0x7fffffffdee8: 0x0000000000000000
Arch:       amd64-64-little
RELRO:      Full RELRO
Stack:      Canary found
NX:         NX enabled
PIE:        PIE enabled
```

- 정상 입력 두 개의 출력 및 경계 입력의 category 변화로 가설을 지지. GDB에서 별도 실행의 category 2→0 변화와 1바이트 쓰기 명령 확인. 출력 사례(category 3→0)와 GDB 사례(category 2→0)는 동일 실행이라고 기록하지 않는다.
- 독립 동적 검증/보호기법 식별 통과. 정확한 GDB 실행 명령 전체, 종료 상태 및 환경 버전은 아직 제출되지 않음. 보호 효과 설명, 수정 설계 및 실제 수정 검증, Day155 D+2와 최종 기록은 남아 있음.
