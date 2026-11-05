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
