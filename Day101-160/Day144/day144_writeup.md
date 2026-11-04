# Day144 — 로더·동적 심볼·GOT/PLT

- 실제 학습일: 2026-11-04 / W04 3/5. 시험 준비로 중단한 Day144부터 재개했다.
- 학습·조건 변경 재검증 종료. 시간은 측정하지 않았다.

## 로더와 호출 경로

Day143의 contains_zero_O0를 분석했다. PT_INTERP는 `/lib64/ld-linux-x86-64.so.2`, NEEDED는 `libc.so.6`, printf 동적 심볼은 UND였다. UND는 실행 파일 안에 정의가 없다는 뜻이며 실행 시 외부 정의를 찾아 연결할 수 있다. LD_DEBUG 실행 로그에서 libc 탐색과 printf 연결이 실행 파일로 제어를 넘기기 전에 일어남을 확인했다. 대상의 BIND_NOW와 일치한다.

실제 호출은 `call 0x1070 <printf@plt>`이며 PLT에서 GOT 슬롯 ELF주소0x3fd0의 값을 읽어 간접 jmp한다. 재배치의 JUMP_SLOT도 printf/0x3fd0을 가리켰다. 슬롯 주소와 슬롯에 저장된 함수 주소는 다르다. PLT의 jmp는 복귀 주소를 추가하지 않으므로 함수 반환은 원래 call 다음 명령으로 이어진다. 이 경로 해석은 정적 분석이다.

## 자체 실험

같은 이름의 자체 공유 라이브러리에 lesson_helper 정의가 있는 경우와 없는 경우를 준비했다.

| 조건 | 관찰 |
|---|---|
| NOW + 정상 라이브러리 | main 진입, helper 반환7, exit0 |
| NOW + 정의 누락 + 호출 생략 | main 전에 symbol lookup error, exit127 |
| lazy + 정의 누락 + 호출 생략 | main 진입 후 정상 종료0 |
| lazy + 정의 누락 + 실제 호출 | main 진입 후 해당 호출에서 오류, exit127 |

지연 연결 프로그램이 한 경로에서 정상 종료해도 미실행 호출의 연결까지 보장하지 않는다.

별도 probe_main.c가 자기 ELF의 JUMP_SLOT을 찾아 메모리 값을 읽었다. lazy에서는 첫 호출 전 실행 파일 내부 주소를 가리켰고, 첫 호출 후 공유 라이브러리 함수 주소로 바뀐 뒤 두 번째 호출에도 유지됐다. NOW에서는 첫 호출 전부터 라이브러리 주소였고 두 호출 동안 같았다. 함수 반환은 양쪽7/7, exit0이었다.

실제 /proc/self/maps에서 lazy 슬롯은 rw-p, NOW 슬롯은 r--p였다. 이번 NOW 빌드의 RELRO 보호와 연결해 GOT 읽기와 덮어쓰기 방지를 구분했다. BIND_NOW만으로 모든 GOT 보호를 단정하지 않는다. 읽기 전용 GOT로도 이미 연결된 함수를 호출할 수 있다. 별도 계측이며 GDB 관찰은 아니다.

## CS·복습·교정

PLT는 코드, GOT는 주소를 담는 데이터이며 실행 파일의 슬롯 주소와 외부 함수의 주소는 별개다. 연결 전 lazy 슬롯을 libc 주소로 취급하면 안 된다고 설명했다. libc 함수 값은 libc 기준 주소 계산의 근거이지 실행 파일 PIE 기준 주소와 혼용하지 않는다.

Day114 실제 ASan 기록의 #0 __asan_memcpy/#1 parse_record:62/#2 main:100을 대조했다. #0은 감지 지점이고 우리 복사 인자를 보려면 #1의 memcpy부터 확인한다. 인자 출처는 상위 호출자 프레임으로 추적한다. 백트레이스 번호가 호출 관계를 나타냄을 설명받고 새 read_packet/handle_client/main 예제에서 호출자를 맞혔다. 오늘 ASan 재실행은 없다.

libc 오프셋과 실제 주소, UND와 런타임 연결, 백트레이스 호출 관계는 도움받아 보완했다. GOT/PLT 역할·call/jmp/ret·Full RELRO의 읽기 허용·연결 시점과 모듈 구분은 문답 및 조건 변경으로 확인했다. 최초 무힌트 전체 정답이나 독립 도구 조작으로 기록하지 않는다.

## 재현 자료와 한계

정적 자료·로더 로그는 이 폴더, 자체 소스·빌드 명령·실행 결과는 binding_demo에 있다. 주요 명령은 file, readelf -lW/-dW/--dyn-syms/-rW, objdump -d -Mintel, LD_DEBUG=libs,bindings이다. 상세 컴파일 옵션은 build_commands.txt와 got_probe_build.txt에 보존했다. 정상·의도된 연결 실패 결과를 구분하며 프로그램 바이너리는 커밋에서 제외한다.

실행은 교사 Linux 컨테이너다. 당시 Day143 O0 바이너리의 해시를 환경 기록에 남긴다. 별도 probe의 주소를 기존 printf 대상의 직접 메모리 관찰로 취급하지 않는다. 다음은 Day145의 실제 GDB break/step/register/memory/map 검증이다.
