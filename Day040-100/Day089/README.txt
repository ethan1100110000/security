Day89 — Stripped 1: entry에서 main 찾기

Target: day89_challenge
Architecture: x86-64 Linux ELF, stripped PIE

Primary goal
- ELF entry point에서 시작해 __libc_start_main 호출 흐름을 추적한다.
- main의 PIE-relative offset을 근거와 함께 복원한다.
- main 이후의 실제 실행 흐름과 main처럼 보이는 decoy 함수를 구분한다.

Required evidence
- readelf 또는 Ghidra에서 확인한 entry point.
- entry에서 __libc_start_main까지의 호출 근거.
- 첫 번째 인자로 전달되는 main 후보 주소/offset 계산.
- main 후보가 맞다는 문자열·호출 흐름 근거.
- objdump 또는 GDB 교차검증 1회.
- 실패 입력 1개와 정상 입력 1개.
- 짧은 write-up: day89_rev.md

Rule
- 디버깅 위치는 먼저 스스로 계획한 뒤 실행한다.
- brute force 금지.
