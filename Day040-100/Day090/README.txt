Day90 — Stripped 2: 함수 경계 복원

Target: day90_challenge
Architecture: x86-64 Linux ELF, stripped PIE

Primary goal
- prologue/epilogue, call target, return/jump, XREF를 함께 사용해 함수 경계를 구분한다.
- 표준 frame prologue가 없는 leaf 함수도 함수로 판정할 수 있어야 한다.
- 함수처럼 보이는 byte pattern이나 caller 없는 후보를 실제 실행 경로와 구분한다.

Required evidence
- entry에서 main 복원.
- 실제 main flow의 함수 역할표.
- 최소 1개 함수의 start/end 주소와 경계 근거.
- 표준 `push rbp; mov rbp,rsp` prologue가 없는 함수 1개 식별.
- 문자열/XREF만으로 이름 붙이면 안 되는 후보 1개 설명.
- GDB 또는 objdump 교차검증 1회.
- 정상 입력 1개, 실패 입력 1개.
- 짧은 write-up: day90_rev.md

CS
- function prologue/epilogue의 목적과 최적화 시 패턴이 달라지는 이유.

Rule
- breakpoint, 입력, 관찰 위치는 먼저 스스로 계획한 뒤 실행한다.
- brute force 금지.
