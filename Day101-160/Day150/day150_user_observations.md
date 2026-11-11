# Day150 — 사용자 관찰·가설·교정 발췌
2026-11-11 / W05 4/5
아래는 대화에서 사용자가 제공한 내용에 근거한 발췌다. 원본 전체 GDB transcript와 로컬 생성 파일은 미첨부.

## 메인: Submit 버퍼 48B → 64B 변경
- Day147 challenge.c를 사용자가 Day150 디렉터리에 복사해 직접 record 크기를 64로 수정, build.sh로 컴파일했다고 보고.
- 예상: 57B 입력은 변경된 64B record 안에 들어가 Canary에 도달하지 않고 정상 종료. 112B read 요청은 그대로라 취약점은 해결되지 않음.
- 사용자 실행: Day150 archive_desk에서 예전 57B 파일을 읽어 `received: 57` / 정상 종료.
- 분석 효율에 대한 사용자 지적: 함수의 전체 어셈블리에서 버퍼 포인터를 설정하는 lea와 Canary 저장을 직접 찾으면 됨. 불필요한 라이브러리 read 내부 추적을 줄임.

## 디버깅 시 혼동과 교정
사용자가 `./challenge.c`를 실행해 Shell의 C 문법 오류를 보고, 소스와 실행 파일을 구분한 후 `./archive_desk` 실행.
셸에서 `run < /tmp/day149_57.bin` 입력해 "Command 'run' not found" → `run`은 GDB 내부 명령이며 셸에서는 `./archive_desk < ...` 실행하는 점 확인.
`b read`에서 처음 멈춘 위치는 stdin의 fgets 내부 libc_read(buf=0x7ffff7e1ab23,nbytes=1)였음; Submit의 read가 아님.
`condition 1 $rdx == 0x70` 안내 후 사용자 제출:
rdi=0x0
rsi=0x7fffffffdf10
rdx=0x70
bt: #0 __GI___libc_read, #1 0x5555555552c2, #2 0x5555555553cf, #3 0x5555555554fe ...
=> Submit용 read 호출 확인. 이 주소는 실행마다 ASLR에 따라 달라질 수 있음.

## 신규 정적 분석
- 사용자가 변경 바이너리의 역어셈블에서 "버퍼 시작은 rbp - 0x50, 카나리값이 저장되는 위치는 rbp - 0x8"이라고 보고.
- 시작부터 Canary까지 0x50-0x8=0x48=72B.
- 메모리 배치: record[64] → padding[8] → Canary.
- Canary 첫 바이트에 도달하는 입력은 73번째.
- 입력 73번째를 0x00으로 두면 검사 통과할 것으로 사용자 독립 예측.

## 신규 입력 실험 — 사용자 원문 출력
ethan@ddengyoon:/mnt/d/security-roadmap/Day101-160/Day150$ ./archive_desk < /tmp/day150_null.bin
Archive desk / local exercise
1 preview / 2 submit / 3 quit
> record> received: 73
1 preview / 2 submit / 3 quit
> ethan@ddengyoon:/mnt/d/security-roadmap/Day101-160/Day150$ ./archive_desk < /tmp/day150_A.bin
Archive desk / local exercise
1 preview / 2 submit / 3 quit
> record> received: 73
*** stack smashing detected ***: terminated
Aborted (core dumped)

입력 생성 방법(교사 안내):
python3:
Path("/tmp/day150_null.bin").write_bytes(b"2\n" + b"A"*72 + b"\x00")
Path("/tmp/day150_A.bin").write_bytes(b"2\n" + b"A"*73)
=> 길이 73 동일, 마지막 값 00/41에 따라 Canary 검사 결과가 다름. 원래 C record[64]를 넘어선 쓰기는 양쪽 모두 존재. 정확한 Canary 변경 바이트 덤프는 이 변경 바이너리에서 제출되지 않았으므로 정적 배치 및 실행 결과로 판단.

## CS: 구조체 Padding/Alignment
코드 구조:
struct Packet { uint8_t type; uint32_t length; uint8_t flags; };
사용자 최초에는 모르겠다고 답했다가 교사에게 구조체 멤버 정렬 설명을 들은 후 `flags` offset8, `sizeof=12`, trailing padding3B를 직접 계산.
사용자 로컬 GCC 실행:
./packet_layout
sizeof: 12
type:   0
length: 4
flags:  8
=> 직접 작성/실행했다고 보고. 정확한 로컬 소스 bytes와 빌드로그는 미첨부, 제공된 교사 코드 기반 참고 소스를 별도 파일로 보존.

## 보안 2회독: Day118 Crash Deduplication
Git의 Day118/write_up.txt와 대조.
사용자 최초 답: "서로 다른 3개 입력이 같은 명령에서 크래시가 발생했으면 같은 취약점에서 걸렸을 가능성이 크지만 하나라고 단정은 비약, 값 이동과 어떤 메모리에 접근하는지 봐야" — 맞음.
조건 변경 예제(손 추적, 사용자 프로그램 실행 아님):
char payload[16]; memcpy(payload,input,len);
A: len20 input_size32 -> stack-buffer-overflow WRITE; 목적지 16보다 4B 초과.
B: len8 input_size4 -> heap-buffer-overflow READ; 원본 4보다 4B 초과.
C: len24 input_size32 -> stack-buffer-overflow WRITE; 목적지 16보다 8B 초과.
사용자 최초 "여러개로 분류?" 후 구체 답: "ac는 payload 이상의 크기로 write해서 범위 밖 쓰기고 b는 input보다 복사하는 len이 커서 범위 밖 읽기임".
=> A와 C는 동일한 종류의 destination length check 누락; B는 별도의 source length check 누락. 입력 세 개가 서로 다른 3개 취약점을 뜻하지 않음. 실제 취약점 두 종류로 묶는 것은 이 예제의 전제하에서 합리적.

## 검증 한계 및 계획표
- 메인 실행·CS struct 실행은 사용자가 직접 보고, 보안 복습은 문답. 독립 무힌트 전체 프로젝트/새 외부 대상 exploit 수행 아님.
- GDB read 조건 설정/컴파일 지침, Padding 개념, fuzzing 반례는 교사의 도움. 독립 해결과 힌트 후 정정 구분.
- GitHub 최신 hanyang-research-plan-2027.xlsx의 Day149/150 상세 행은 Github connector가 XLSX 바이너리 UTF-8 오류로 반환해 직접 대조하지 못함. 나중에 해당 셀을 확인해 추가 필수 항목이 있으면 보완해야 함.
