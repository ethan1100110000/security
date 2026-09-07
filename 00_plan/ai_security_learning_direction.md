# AI-assisted Security Learning Direction

## 목적

이 문서는 보안/리버스 엔지니어링 학습의 장기 방향을 정의한다.
기존 `00_plan/progress_log.md`는 Day별 진행 상태와 다음 작업의 기준 파일로 유지하고,
이 문서는 AI 성능이 빠르게 향상되는 환경에서 무엇을 직접 익히고 AI를 어떻게 사용해야 하는지에 대한 상위 원칙으로 사용한다.

---

## 핵심 목표

학습 목표를 **"AI보다 빨리 정답을 찾는 사람"**으로 두지 않는다.
최종 목표는 다음과 같다.

> **AI가 생성한 분석을 이해하고 검증하며, 잘못된 가정을 찾아내고, AI를 이용해 더 어려운 바이너리와 취약점을 분석할 수 있는 보안 연구 역량을 만든다.**

따라서 어셈블리, 메모리, ABI, ELF, 디버거, 취약점 원리 같은 기초를 생략하지 않는다.
AI가 높은 확률로 정답을 낼 수 있는 문제라도 사용자가 그 결과의 타당성을 검증할 수 없다면 해당 주제를 제대로 익힌 것으로 보지 않는다.

---

## 학습 원칙

### 1. 직접 분석 능력을 먼저 만든다

새로운 개념을 처음 배울 때는 AI가 최종 답을 먼저 공개하지 않는다.

기본 순서:

1. 사용자가 바이너리/소스/Ghidra/GDB 결과를 직접 관찰한다.
2. 사용자가 data flow, 조건, 취약점 후보, 다음 디버깅 지점을 먼저 제안한다.
3. AI는 틀린 가정, 빠진 관찰 포인트, 다음 검증 방법을 질문 또는 힌트로 제시한다.
4. 실제 GDB/raw memory/objdump/Ghidra/ASan 등의 증거로 확인한다.
5. 성공 케이스뿐 아니라 실패 케이스도 확인한다.
6. 마지막에 짧은 write-up으로 원인을 설명한다.

정답을 맞힌 것보다 **왜 맞는지 증명할 수 있는가**를 더 중요하게 본다.

### 2. AI 출력은 가설로 취급한다

AI가 다음과 같은 결론을 제시해도 즉시 사실로 받아들이지 않는다.

- overflow 가능
- 특정 함수가 parser/checker 역할
- libc/PIE leak 가능
- 특정 branch가 우회 가능
- 특정 sink까지 입력이 도달함
- exploit 가능

각 결론은 최소 하나 이상의 독립적인 증거로 확인한다.
가능하면 정적 분석과 동적 분석을 교차 검증한다.

예:

- Ghidra decompile ↔ Listing/objdump
- AI pseudocode ↔ 실제 register/memory 변화
- 예상 crash ↔ ASan/GDB crash 위치
- 예상 control flow ↔ breakpoint/coverage
- 계산한 주소 ↔ runtime mapping

### 3. AI와 경쟁하지 않고 AI를 증폭기로 사용한다

반복적이고 기계적인 작업은 점차 AI에 맡긴다.

예:

- 긴 함수의 1차 pseudocode 생성
- 함수 후보 분류
- 반복되는 bit operation 정리
- crash 로그 군집화
- fuzzing 결과 요약
- 여러 가설 생성
- 분석 기록 정리

대신 사용자는 다음 판단을 담당한다.

- 어떤 가설을 먼저 검증할지
- 어떤 관찰이 결정적인 증거인지
- AI 분석이 어떤 전제를 놓쳤는지
- 실제 프로그램의 앞단 validation 때문에 취약점이 unreachable한지
- primitive가 실제 exploit으로 연결 가능한지
- false positive인지 real bug인지

### 4. Source-level 정답보다 Binary-level 증거를 우선한다

소스가 제공된 toy challenge에서도 최종 이해는 바이너리에서 확인한다.

중요한 항목:

- 실제 compiler output
- calling convention
- register와 stack 사용
- relocation/PLT/GOT
- PIE/ASLR 적용 후 runtime address
- allocator metadata
- sanitizer가 보고한 실제 fault
- optimizer 때문에 소스와 달라진 흐름

소스는 이해를 돕는 자료이지 최종 증거가 아니다.

---

## 현재 단계에서의 적용

현재 Day115까지 parser harness, ASan, showmap, stdin/file harness 비교를 진행했다.
Day116 이후 fuzzing/crash triage 구간부터 AI-assisted workflow를 점진적으로 추가한다.

단, 기존의 직접 분석 루틴을 제거하지 않는다.

### Day116 이후 기본 루틴

#### Phase A — Human first

사용자가 먼저 다음을 말한다.

- 관찰한 현상
- 가장 가능성이 높은 원인
- 필요한 추가 증거
- 다음에 실행할 명령/브레이크포인트/실험

AI는 즉시 정답을 주지 않고 해당 계획의 허점을 점검한다.

#### Phase B — Evidence

실제로 다음 중 필요한 방법으로 확인한다.

- GDB
- Ghidra
- objdump/readelf/nm
- ASan/UBSan
- AFL++ coverage/crash
- raw memory
- 최소 재현 입력

#### Phase C — AI second opinion

증거를 확보한 뒤 AI에게 독립적으로 분석하게 한다.

비교 대상:

- 사용자 분석과 AI 분석이 같은가?
- 서로 다른 가정은 무엇인가?
- AI가 놓친 입력 조건이 있는가?
- 사용자가 놓친 control/data flow가 있는가?

#### Phase D — Adversarial review

중요한 문제에서는 AI 분석을 일부러 반박한다.

질문 예:

- 이 crash가 정말 exploitable한가?
- caller에서 이미 길이 검사를 하지 않는가?
- 해당 pointer를 실제로 attacker가 제어할 수 있는가?
- ASLR/PIE/RELRO/NX/Canary 때문에 primitive가 끊기지 않는가?
- 입력이 실제 프로그램 경로에서도 해당 parser까지 도달하는가?
- sanitizer-only crash는 아닌가?

#### Phase E — Write-up

최종 기록에는 최소한 다음을 포함한다.

1. root cause
2. 입력이 bug까지 도달하는 data flow
3. 재현 증거
4. 실패하거나 틀렸던 가설
5. exploitability 판단
6. AI 분석과 사람 분석이 달랐던 부분이 있다면 그 차이

---

## 단계별 성장 목표

### Stage 1 — AI 없이 기본기를 검증할 수 있음

- x86-64 기본 명령과 flag
- stack/register/memory
- 함수 호출과 ABI
- ELF/PIE/PLT/GOT/RELRO
- heap allocator 기본 구조
- GDB/Ghidra/objdump 사용
- BOF/UAF/OOB 등 기본 memory corruption
- fuzzing/harness/coverage/crash 기본

이 단계에서는 작은 challenge를 AI 없이도 분석할 수 있어야 한다.

### Stage 2 — AI 분석을 검증할 수 있음

AI에게 바이너리 또는 분석 결과를 주고 받은 설명에서 다음을 판별할 수 있어야 한다.

- 사실
- 추론
- 확인되지 않은 가정
- 잘못된 결론

AI의 설명을 그대로 믿지 않고 필요한 실험을 설계할 수 있어야 한다.

### Stage 3 — AI를 이용해 분석 범위를 확장함

혼자라면 시간이 오래 걸리는 대상에서 AI를 사용한다.

- 더 큰 stripped binary
- 여러 parser/state machine
- 복수 crash triage
- 복잡한 data flow
- coverage 비교
- 여러 exploit primitive 후보

AI에게 단순 정답을 요구하는 대신 서로 다른 가설을 생성하게 하고 사용자가 검증 우선순위를 결정한다.

### Stage 4 — AI-assisted security research

최종적으로는 다음 workflow를 목표로 한다.

```text
Target
  ↓
Recon / static triage
  ↓
AI hypothesis generation
  ↓
Human prioritization
  ↓
Dynamic verification
  ↓
Fuzzing / crash collection
  ↓
AI-assisted triage
  ↓
Manual root-cause analysis
  ↓
Exploitability assessment
  ↓
PoC / exploit
  ↓
Independent verification + write-up
```

이 단계에서 AI는 분석 속도를 높이는 도구이며, 최종 판단과 증거의 책임은 사용자에게 있다.

---

## 앞으로 문제 난이도를 올리는 기준

같은 개념의 단순 반복 횟수를 무한히 늘리지 않는다.
다음 조건을 만족하면 더 어려운 문제로 이동한다.

- 원리를 자신의 말로 설명 가능
- 소스 없이 바이너리에서 핵심 흐름을 찾을 수 있음
- GDB로 실제 상태를 확인할 수 있음
- 의도적으로 만든 실패 케이스의 원인을 설명할 수 있음
- AI가 잘못된 분석을 제시했을 때 검증 방법을 제안할 수 있음

반대로 문제를 우연히 풀었더라도 위 조건을 충족하지 못하면 해당 개념을 완료한 것으로 간주하지 않는다.

---

## ChatGPT의 역할

ChatGPT는 기본적으로 **교사 + 분석 파트너 + 리뷰어** 역할을 한다.

- 새로운 개념에서는 사용자가 먼저 사고하도록 유도한다.
- 정답이나 exploit 전체를 필요 이상으로 먼저 공개하지 않는다.
- 사용자의 주장에 자동으로 동의하지 않는다.
- 틀린 가정이 있으면 근거를 요구한다.
- 성공 후에는 다른 증거로 교차 검증하게 한다.
- 일정 수준 이후에는 실제 AI-assisted reverse engineering 방식도 훈련한다.
- 필요할 경우 AI 분석 자체를 검증하는 문제를 출제한다.

학습의 목표는 ChatGPT에게 의존하는 것이 아니라,
**ChatGPT를 사용할 때와 사용하지 않을 때 모두 분석의 타당성을 판단할 수 있는 능력**을 만드는 것이다.

---

## 장기 판단 기준

AI 성능이 더 올라가더라도 다음 질문에 스스로 답할 수 있다면 학습은 의미가 있다.

- AI의 분석이 왜 맞는가?
- 어떤 증거가 부족한가?
- 어느 부분이 단순 추측인가?
- 이 bug는 실제 경로에서 reachable한가?
- 실제 공격 primitive가 존재하는가?
- mitigation을 고려한 뒤에도 exploitable한가?
- AI가 놓친 조건을 어떻게 찾을 것인가?

궁극적으로 목표는 **문제를 대신 풀어주는 AI 사용자**가 아니라,
**AI와 도구를 활용해 더 깊은 보안 문제를 검증하고 해결할 수 있는 연구자**가 되는 것이다.
