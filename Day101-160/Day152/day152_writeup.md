## Day152 — W06 1/5 / x86-64 호출 규약과 스택 프레임 (진행 중)

### 2026-11-13 메인 실측 1: 함수 진입 상태
- 교사가 작성해 Git에 추가한 [`call_frame_probe.c`](call_frame_probe.c) 컴파일 설정: `gcc -O0 -g -fno-omit-frame-pointer call_frame_probe.c -o call_frame_probe`.
- 사용자의 이론 예상: 정수 인자 1~6은 RDI,RSI,RDX,RCX,R8,R9; 함수 시작 `$rsp`에는 반환 주소. 처음 `call`에서 `rsp+8`로 오해했지만 교사가 `call` 시 스택은 -8B, `ret` 시 +8B라는 방향을 설명.
- 사용자 `break *inspect` 후 GDB/Pwndbg 실제 출력:

```
rdi 0xb
rsi 0x16
rdx 0x21
rcx 0x2c
r8  0x37
r9  0x42
rsp 0x7fffffffdf78
rbp 0x7fffffffdf90
x/4gx $rsp
0x7fffffffdf78: 0x0000555555555259 0x0000000000000000
0x7fffffffdf88: 0x0000000000000000 0x0000000000000001
```

- 인자값은 각각 11/22/33/44/55/66으로 C 호출 실인자와 일치. 함수 첫 명령이 실행되기 전 상태라는 전제에서 `[rsp] = 0x555555555259`는 반환 주소로 해석. 주소가 실제 `main` 내 `call inspect` 다음 명령 위치인지 디스어셈블로 추가 대조 예정. `rbp-rsp = 0x18=24B`이지만 이는 **현재 inspect의 프레임 크기를 의미하지 않는다**. 함수 프롤로그 전에 관찰된 `rbp`는 호출자의 프레임 포인터 값일 수 있음.
- 목표: `disassemble inspect` 및 `x/i *(void **)$rsp`로 반환주소의 명령 위치 확인, 함수 프롤로그 전후의 RSP/RBP 변화 분리.
- 아직 CS 호출 규약 무힌트 반례, Day040~050 이전 스택/ROP Git 리뷰, Write-up 최종 검증은 미실시. **Day152 미완료.**
