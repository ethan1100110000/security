# Day155 — FINAL W06 3/4 — PIE × ASLR 2×2 주소 재배치 실험

> 수행일: 2026-11-18 (KST). 로컬 교육용 실험 소스: [`addr_probe.c`](addr_probe.c), 빌드: [`build.sh`](build.sh), 관찰 스크립트: [`measure.py`](measure.py). 사용자 WSL 실측·무힌트 계산과 교사 제작 코드/가상 복습 상황을 구분한다.

## 문제와 설계
- 동일한 C 소스로 **PIE OFF / ON** 두 바이너리를 만들고, 별도 프로세스 단위로 **ASLR OFF / ON**을 전환한다. 4조건마다 10회 독립 실행하여 총 40회 관측.
- `main`, `helper`, `stack`, `libc_puts`, `main_minus_helper`를 출력해 절대주소 변동과 상대 오프셋 불변 여부를 구분한다.
- 공통 컴파일 옵션 `-O0 -g -Wall -Wextra`. PIE OFF: `-fno-pie -no-pie`, PIE ON: `-fPIE -pie`. 주소 출력용으로 `dlsym(RTLD_DEFAULT,"puts")` 사용, `-ldl` 링크.
- ASLR ON: 보통 실행. ASLR OFF: `setarch $(uname -m) -R`으로 실행당 `ADDR_NO_RANDOMIZE` 요청. 사용자 환경에서 이 명령은 허용되어 성공했다고 요약 출력으로 확인. 시스템 전체 ASLR 설정을 바꾸지 않음.

## 먼저 세운 가설 (학생 독립)
- ASLR ON일 때 **PIE OFF**의 `main`/ `helper`는 고정, `stack`/ `libc_puts`는 변동할 것으로 예상.
- ASLR ON일 때 **PIE ON**이면 `main`/ `helper`도 변동할 것으로 예상.
- 같은 바이너리의 `main - helper`는 로드 기준이 달라져도 고정.
- ASLR OFF에서는 PIE 플래그와 무관하게 이번 실험에서 출력한 각 주소가 재실행 간 고정될 것으로 예상.
- 위 가설은 **측정 전에 제출**했으며 4조건 요약과 모두 일치함.

## 실측 — 10회당 서로 다른 값의 개수

| PIE | ASLR | main | helper | stack | libc_puts | main_minus_helper |
|---|---|---:|---:|---:|---:|---:|
| OFF | OFF | 1 | 1 | 1 | 1 | 1 |
| ON | OFF | 1 | 1 | 1 | 1 | 1 |
| OFF | ON | 1 | 1 | 10 | 10 | 1 |
| ON | ON | 10 | 10 | 10 | 10 | 1 |

학생의 요약 출력 원문:
```text
$ python3 measure.py --aslr on --runs 10
no_pie trials=10 unique-values: {'main': 1, 'helper': 1, 'stack': 10, 'libc_puts': 10, 'main_minus_helper': 1}
pie trials=10 unique-values: {'main': 10, 'helper': 10, 'stack': 10, 'libc_puts': 10, 'main_minus_helper': 1}

$ python3 measure.py --aslr off --runs 10
saved: measure_aslr-off.csv
no_pie trials=10 unique-values: {'main': 1, 'helper': 1, 'stack': 1, 'libc_puts': 1, 'main_minus_helper': 1}
pie trials=10 unique-values: {'main': 1, 'helper': 1, 'stack': 1, 'libc_puts': 1, 'main_minus_helper': 1}
```

**해석:** PIE의 존재는 실행 이미지를 상대주소 기반으로 재배치 가능하게 하는 특성이다. 런타임 ASLR이 활성화돼야 이번 관측과 같은 매번 달라지는 코드 로딩 주소가 나타난다. PIE OFF여도 스택·공유 라이브러리 주소는 ASLR의 영향으로 변할 수 있다. 주소 변화는 **각 조건의 10회 표본에서** 관찰했을 뿐 모든 부팅/운영체제/프로세스에 대한 절대적 법칙으로 일반화하지 않는다.

## CS — PIE 로딩 기준과 함수 간 오프셋을 독립 계산
학생이 직접 제출한 PIE 바이너리 **한 프로세스**의 출력과 `nm` 결과:
```text
main=0x5f4c76b3519c helper=0x5f4c76b35189 stack=0x7ffd90f0fc40 libc_puts=0x7e4177880e10 main_minus_helper=0x13
$ nm -n addr_pie | grep ' main$'
000000000000119c T main
```
- PIE 로딩 기준 계산: `0x5f4c76b3519c - 0x119c = 0x5f4c76b34000`.
- ELF 내 `helper` 오프셋 역산: `0x5f4c76b35189 - 0x5f4c76b34000 = 0x1189`.
- 두 함수 간 차: `0x119c - 0x1189 = 0x13`; 런타임 `main_minus_helper` 관측값과 일치.
- 학생이 `main` runtime, `main` symbol offset, base, `helper` offset, `0x13` 검증까지 **무힌트로 직접 계산**함.

## 주소 누출이 없는 조건과 정량 해석 한계
- 이 실험 프로그램이 출력한 주소는 **측정용으로 의도적으로 제공한 런타임 포인터**다. 실제 바이너리에서 해당 주소 출력이 없는 **no-leak** 상황과 혼동하지 않는다.
- PIE ON + ASLR ON에서 `main=base+0x119c`, `helper=base+0x1189`이고 `base`를 모른다면 `helper`의 **절대주소를 오프셋만으로 결정할 수 없음**을 학생이 스스로 설명. 같은 로딩 이미지의 주소 하나와 대응하는 ELF 오프셋을 안다면 기준 주소 역산이 가능함.
- ASLR ON/PIE ON의 `main` 주소는 표본 10개 중 10개가 서로 다르므로 각 샘플의 `main`에서 **같은** ELF 오프셋 `0x119c`를 뺀 페이지 정렬 베이스 역시 표본에서 **10개** 존재한다.
- 이는 **10회 표본에서 관측된 다양성**일 뿐 전체 가능한 베이스의 수, 선택 분포, 비트 엔트로피, 임의 추측 성공 확률을 의미하지 않는다. 학생이 `1/10` 확률 추정은 성립하지 않는다고 답함. 따라서 실제 성공 확률 수치를 만들지 않음.

## Day040~050 복습 — 후보 주소가 정렬된 것만으로 PIE 누출 판정 불가
과거 학생 Git [Day043](../../Day040-100/Day043/write_up.txt)에서는 fsb로 얻은 PIE 후보에서 함수 오프셋을 빼고 페이지 정렬과 실행 맵 영역을 교차 확인했다. [Day050](../../Day040-100/Day050/write_up.txt)에서도 PIE 후보·카나리를 누출하여 바이너리 내 `/bin/sh` 호출 함수의 런타임 주소를 계산했다.

**변경된 가상 조건:** `main` ELF 오프셋 `0x119c`, 누출 후보 `A=0x55555555619c`, `B=0x7ffff7c0119c`.
```text
0x555555555000 - 0x555555556000 r--p /tmp/target
0x555555556000 - 0x555555557000 r-xp /tmp/target
0x7ffff7c00000 - 0x7ffff7c26000 r--p /lib/libc.so.6
0x7ffff7c26000 - 0x7ffff7dac000 r-xp /lib/libc.so.6
```
- `A-0x119c=0x555555555000`, `B-0x119c=0x7ffff7c00000`으로 두 값 모두 페이지 정렬이다.
- 학생 무힌트 정답: **A를 main 후보로 선택**. 이유는 `main`이 libc 함수가 아니라 **target 바이너리 자체의 함수**이기 때문. A는 target의 `r-xp` 맵에 존재, B는 libc의 `r--p` 맵에 위치. 정렬만으로 포인터가 대상 이미지에 속한다는 사실을 증명하지 못한다.
- 따라서 **ELF 심볼 오프셋 + 메모리맵의 파일 출처/권한 + 실제 주소 범위**를 함께 대조해야 한다. 가상 조건은 별도 실습 결과로 오해하지 않는다.

## Day153 D+2 장기기억 복습 — 교정 과정 보존
Day153 완료 2026-11-16 → 2026-11-18 D+2.
- 바뀐 조건 `a=63,b=7,data=0x42×63`, 목적지 기준 `b:+0x3c..3d`, `a:+0x3e..3f`. 학생의 **최초 회상**에서는 `a` 2B/`b` 1B 덮는다고 **반대로 답했고**, saved RBP/return 미도달은 맞음. `mode=b`, `sum=최대48B 합계` 의미를 잊었다고 직접 말함.
- 교사에게 경계·출력 의미 힌트를 받은 뒤 **`a=0x0042`, `b=0x4242`, `mode=16962`, `sum=3168`**을 정확하게 산출. 처음부터 무힌트로 성공했다고 기록하지 않음.
- 후속 변경 조건 `a=61`, `b=0x1234`, `0x42×61`에 대한 학생 독립 정답: `a=0x003d` 불변, `b=0x1242` 하위 1B만 변경. **교정 후 새 문제 무힌트 재검증 통과**.
- 시간적 순서 관련 학생 독립 설명: `a`가 63으로 길이 검사를 통과한 **후에** `memcpy` 쓰기로 변수 오염이 발생하므로 이전 검사가 소급 무효가 되는 것은 아님. 이후 루프 조건 등에 오염된 변수값을 사용할 수 있음.

## 결론 및 증거 게이트
- **메인 수행:** 4조건 × 10회 = 총 40회 학생 로컬 실행 요약, PIE/ASLR 독립 예측 대조, 런타임 로드베이스/오프셋 계산, 누출 없을 때의 정보 부족과 표본 10개의 추론 한계, 2×2 표 작성.
- **CS 수행:** PIE/ASLR 역할 분리와 학생의 실제 주소/심볼 오프셋 계산 완료.
- **복습 수행:** Day043/050의 PIE 주소 후보 출처 검증, Day153 D+2 초기 오답과 힌트 후 재검증 과정을 구분하여 완료.
- **기록:** 소스·빌드·측정 스크립트·실측 요약·표·원시 출력 1개·계산 결과를 본문에 Git 기록.
- **원본 증거 검증 완료:** 사용자 Git 커밋 `b4e92fb9a8f1fc0df5878ed0b89e22354e9daac4` (`study: Day155 raw PIE ASLR measurements`)에 [ASLR ON 원본 20행](measure_aslr-on.csv)과 [ASLR OFF 원본 20행](measure_aslr-off.csv) 추적됨. 각 CSV의 `no_pie` 10행, `pie` 10행 및 순번 1~10 확인.
- **실제 CSV 검증 결과:** ON: `no_pie` main/helper 1종, stack/libc 10종; `pie` main/helper/stack/libc 10종. OFF: `no_pie`, `pie` 각 모든 주소 1종. 모든 행에서 `runtime_main-runtime_helper=main_minus_helper=0x13`. PIE ON에서 `main-0x119c`를 계산한 base 고유 개수는 ASLR ON=10종, OFF=1종. 요약 보고 내용과 원본 데이터 일치.
- **최종 판정: Day155 완료 (2026-11-18).** 메인 실측 4조건×10회, CS 무힌트 오프셋 계산, Day040~050 변형 복습과 Day153 D+2 교정 후 독립 재검증, 원시 CSV 및 write-up Git 증거를 모두 확보함.
