# Day146 — 7바이트 파서 경계와 ASan 재검증
2026-11-05 / W04 5/5

사용자가 로컬 WSL에서 Day114 파서를 ASan으로 빌드·실행하고, Day146 복사본을 수정해 재검증했다. 명령 안내는 교사가 제공했다.

## 실습
입력은 TPAR + version=1 + command=1 + length + 본문이다. input_size=24/length=17은 원본 본문 길이 검사에서 17>17이 거짓이라 통과한다. 사용자는 처음 거부라고 답했지만 대입 후 통과 및 16바이트 목적지의 범위 밖 쓰기를 설명했다.
원본 실행에서 사용자가 확인한 보고서는 stack-buffer-overflow / WRITE of size 17 / [32,48) payload / offset48 초과였다. 17바이트 복사 요청 중 목적지 밖은 1바이트이며, 보고서가 17바이트 모두 밖에 쓰였다는 뜻은 아니다.

사용자가 제안한 수정:
```c
char payload[16] = {0};
if (length > sizeof(payload)) {
    return reject(state, "payload too large");
}
memcpy(payload, input + 7, length);
```

최초에는 수정 없이 컴파일해 16 정상/17 기존 오류를 재현했다. 이를 수정 전 결과로 구분하고 소스 저장 후 재컴파일했다. 재실행 제출 출력:
```text
length=16, exit=0
AAAAAAAAAAAAAAAA
state=6
length=17, exit=1
reject at state 5: payload too large
```
length16/본문15라는 조건 변경에서는 원본 부족 검사 메시지 truncated payload를 사용자가 맞혔다. 이 사례는 문답 검증이며 실제 실행 출력은 제출되지 않았다.

## CS·복습
PT_LOAD 예제 Offset2000/VirtAddr5000/FileSiz80/MemSizA0/RW에서 5090은 0채움 부분이라고 답했다. 5050의 파일 위치를 처음 50이라 답해 상대거리와 파일 오프셋을 구분한 뒤 2050으로 교정했다. 역방향 조건 변경인 파일2060→ELF5060은 스스로 맞혔다. PT_LOAD에 없는 debug_info는 실행용 로딩 대상이 아님을 확인했다. 새 readelf 실행은 아니다.

test eax,eax / jne failed에서 반환 -1이면 점프한다고 답했으나 AND를 '같으면1'로 설명했다. FFFFFFFF & FFFFFFFF는 FFFFFFFF이고 비영 결과라 ZF0임을 교정했다. 새 값2의 AND 결과는2라고 맞혔다. test는 결과를 레지스터에 저장하지 않고 플래그를 갱신한다. 실제 파서의 역어셈블을 새로 관찰한 것이 아니라 설명용 명령이다.

## 기록·한계
이미 확인한 원리를 다시 종합 설명시키는 반복 요청은 사용자 지적에 따라 생략했다. 최초 완전 무힌트 숙달로 기록하지 않는다. 이전 ABI/파서/ASan 약점을 실제 입력과 조건 변경으로 복습했다. 시간 미측정.
근거는 사용자 제출 발췌 [day146_user_observations.txt](day146_user_observations.txt). 사용자 로컬의 수정 소스와 전체 ASan/테스트 파일은 아직 원격에서 확인하지 않았으므로 이 커밋에 포함한 것으로 주장하지 않는다.
다음은 Day147 / W05 1/5 — 새 로컬 CTF 입력 경로·보호기법 진단.


## 2026-11-12 누락 필수 과제 보완 — 파서 원인·검사·검증 독립 설명
**계획표 기준:** Day146 / W04 5/5 / 7바이트 파서 OOB·용량 검사·ASan 원인 재검증과 무힌트 설명. 2026-11-05 기존 실측 데이터를 재사용하고 별도 구술 문답을 진행했으며 오늘 새 ASan 실행은 하지 않았다.

**사용자 첫 독립 설명**
1. 목적지 `payload[16]`에 17B를 복사해서 범위 밖 쓰기가 발생하며, `length > sizeof(payload)` 검사가 누락된 것이 원인이라고 설명했다.
2. `if(length > sizeof(payload))`의 목적지 검사를 제시했지만 **원본 데이터 길이 검사는 처음 설명에서 누락**했다.
3. 수정 전에는 16B 정상/17B ASan overflow, 수정 후 16B 허용/17B 검사를 통한 거부라고 설명했다. '17B 정상종료'라는 표현은 정확히는 프로그램이 `exit=1`로 의도적으로 입력을 거부한 것이며, `exit=0` 정상 처리와 다름을 교정했다.
4. 특정 크기 취약점 하나의 수정·제한된 사례 검증은 전체 입력 공간과 다른 취약점까지 안전하다는 증명이 아니므로 추가 분석/퍼징 필요성을 말했다.

**힌트 후 직접 경계 재검증**
- 교사가 헤더를 제외한 원본 길이 검사가 빠졌다고 지적하고 `input_size=10, length=8` 조건을 제시했다.
- 사용자는 `input_size-7 < length`이면 거부해야 한다고 직접 조건식을 답했다(3B 본문에서 8B 복사는 원본 범위 밖 읽기).
- 교사가 `input_size=5,length=8` 반례 및 `size_t` 뺄셈 질문을 제시하자, 사용자는 **아주 큰 양수로 언더플로우**된다는 점과 **빼기 전에 경계를 먼저 확인할 필요**가 있음을 설명했다.
- 정밀 교정: `input_size-7`을 계산한 *후* 양수인지 검사하면 이미 unsigned underflow가 발생하므로, **빼기 전에** `if (input_size < 7) return ...;` 순서로 검사한다. 또한 `input_size==7,length==0`은 원본 범위 조건상 허용될 수 있으므로 `>7`을 강제하지 않는다.

**최종 안전 조건(의미 정리, 새 수정 실행 아님)**
```c
if (input_size < 7) return reject(state, "truncated header");
if (length > input_size - 7) return reject(state, "truncated payload");
if (length > sizeof(payload)) return reject(state, "payload too large");
memcpy(payload, input + 7, length);
```
이 순서는 `length`가 `size_t` 등 비음수이고 `input`이 실제 `input_size`만큼 유효한 읽기 가능한 객체를 가리킨다는 **전제**에서 두 크기 검사를 구성한다. 실제 코드에서 다른 정수형을 쓰면 음수 검사를 선행해야 한다.

**독립성/한계 판정:** 목적지 초과 원인·수정과 안전성 검증 한계는 사용자 독립 설명. 원본 크기 검사는 누락 후 힌트로 보완, 언더플로우도 교사의 반례 제시 후 사용자 스스로 확인. 최초 완전무힌트 네 항목 전부 통과는 아님을 기록. **Day146 누락 항목 보완 완료**, 최초 설명의 오류와 도움받은 부분은 감추지 않는다. 원래 Day146 ASan 실측 파일 재실행 없음.
