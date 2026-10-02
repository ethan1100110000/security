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
```

| PoC | 파일 크기 / Length | 최초 잘못된 연산과 bug class | 일반 빌드 증상 | 확인된 추가 근거 |
|---|---|---|---|---|
| `poc_len17.bin` | 24바이트 / 17 | `payload[16]`을 1바이트 넘는 `memcpy`; 스택 OOB write | `exit=0` | ASan이 `stack-buffer-overflow`를 검출하고 `exit=134`로 종료했다. |
| `poc_len64.bin` | 84바이트 / 64 | 같은 버퍼를 48바이트 넘는 `memcpy`; 같은 스택 OOB write | 3회 모두 SIGSEGV, `exit=139` | GDB에서 복사로 손상된 `input` 포인터를 이후 다시 읽다가 죽는 흐름을 확인했다. 이 입력의 별도 ASan 실행 결과는 위 근거 문서에 기록돼 있지 않다. |

두 PoC는 일반 빌드에서 서로 다른 결과를 보였지만, 목적지 크기 검증 누락과 최초 범위 밖 쓰기 연산이 같다. 따라서 같은 root cause의 서로 다른 증상으로 분류한다. `poc_len64.bin`의 SIGSEGV 발생 위치는 손상된 포인터를 다시 읽은 곳이고, 메모리 손상을 처음 만든 곳은 그보다 앞선 `memcpy`다.

빠졌던 목적지 검사는 `length <= sizeof(payload)`에 해당한다. 이 검사는 위험한 `memcpy` 전에 수행해야 한다.

## TinyXML-2에 적용할 판정 절차

새 crash 파일이 생기면 우선 입력 파일의 크기와 SHA-256, 사용한 바이너리의 SHA-256, 입력 방식과 실행 명령을 기록한다. 같은 파일을 단독 실행해 종료 코드와 signal을 확인하고, sanitizer 보고서 또는 GDB에서 최초 잘못된 접근, 대상 객체, 접근 방향과 크기를 조사한다.

그 증거로 OOB, UAF, assertion failure 등의 분류를 결정한다. 같은 종료 코드나 같은 crash 위치만으로 동일한 root cause라고 확정하지 않는다. 근거가 부족하면 triage queue에서 `원인 미분류`로 유지한다. 외부 입력에 의한 assertion 중단은 정상적인 XML 문법 오류 반환과 구분하고, 재현 조건과 서비스 영향을 확인한다.

## 결론과 한계

이번 Day129 작업은 기존 증거를 이용한 bug class 분류 연습이다. TinyXML-2에서 새 crash 후보를 수집하거나 취약점을 확인한 결과는 아니다. Day117의 두 PoC는 스택 OOB write라는 하나의 원인으로 묶이지만, 일반 빌드의 정상 종료 여부만으로 메모리 안전성을 판단할 수 없음을 보여준다.

CS 정리: crash collection은 후보 입력과 실행 조건을 보존하는 단계다. Crash classification은 단독 재현과 메모리 접근 증거를 바탕으로 오류의 종류를 판단하는 단계다. `exit=134`는 흔히 SIGABRT의 셸 종료값이지만, 그 값만으로 assertion failure라고 분류할 수 없다. Day117의 Length 17은 ASan의 `stack-buffer-overflow` 보고서가 분류 근거다.
