# Day129 — Crash classification

## 대상과 범위

Day127의 TinyXML-2 CLI 퍼징 결과에서 저장된 crash와 hang 후보는 각각 0건이었다. 따라서 이번에는 TinyXML-2의 새 취약점을 분류하지 않는다. 대신 원인 분석이 끝난 Day117 toy parser의 `poc_len17.bin`과 `poc_len64.bin`을 비교 사례로 사용해 bug class와 crash 증상을 구분한다.

근거 자료: [Day117 재현 기록](../Day117/write_up.txt), [Day120 종합 분석](../Day120/day120_fuzzing.md), [Day128 triage queue](../Day128/triage_queue.txt).

## 분류 기준

| 분류 | 확인할 핵심 증거 | 주의할 점 |
|---|---|---|
| OOB read | 객체의 유효 범위 밖에서 읽은 주소, 크기, 소스 위치 | 범위 밖 읽기가 곧바로 정보 유출을 뜻하지는 않는다. |
| OOB write / buffer overflow | 객체의 유효 범위 밖에 쓴 주소, 크기, 최초 쓰기 연산 | 나중에 발생한 crash 위치와 최초 메모리 손상 위치가 다를 수 있다. |
| UAF read/write | 객체가 해제된 시점과 이후 같은 객체에 접근한 위치 | 접근 인덱스가 원래 크기 안에 있어도 해제 후 접근은 잘못이다. |
| Assertion failure | 실패한 조건, 중단 위치, 호출 경로 | assertion에서 중단됐다는 사실만으로 OOB나 UAF가 발생했다고 단정하지 않는다. |

`stack-buffer-overflow`나 `heap-use-after-free` 같은 sanitizer 오류 이름을 기록할 때는 READ/WRITE 방향과 접근 크기도 함께 확인한다. `SIGSEGV`나 `exit=139`는 종료 증상이며, 그것만으로 bug class를 결정하지 않는다. 여기서 buffer overflow는 버퍼 경계를 넘는 쓰기를 뜻하며 정수 오버플로와 구분한다.

## Day117 사례 분류

Day117의 입력 형식은 `TPAR → Version → Command → Length → Payload`다. 원본 입력에 `Length`바이트가 있는지는 검사했지만, 고정 크기 `payload[16]`에 그만큼 복사해도 되는지는 검사하지 않았다. 최초 잘못된 연산은 `parse_record()`의 다음 복사다.

```c
memcpy(payload, input + 7, length);