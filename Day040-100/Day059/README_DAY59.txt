Day059 Mini Exam 2

Binaries:
- day59_p1
- day59_p2

Rules:
- Do not use source code. Treat both as stripped external binaries.
- Put your notes, exploit scripts, gdb logs, and final write-up under Day040-100/Day059.
- Final write-up filename: exam_day59.md

Required analysis order:
1. checksec / file / readelf -h
2. Input point discovery
3. Data flow to risky sinks
4. Leak/write/BOF candidates
5. Protection-specific exploit strategy
6. gdb verification
7. exploit
8. failure-case notes

Pass condition:
- P1: obtain code execution or demonstrate a complete verified exploit path.
- P2: reach the unlock path or demonstrate a complete verified exploit path.

Reminder:
- Do not start from payload generation.
- Start from root cause, required leaks, address calculations, and validation.
