# Day079 Hard — Menu UAF leak + BOF

프로그램이 libc 주소를 자동으로 출력하지 않는다.

직접 메뉴를 조작해 heap 배치를 만들고, free 후 남은 dangling pointer를 `show`로 읽어
unsorted-bin 포인터를 획득한 뒤 별도의 BOF 기능과 연결하는 실습이다.

완성 exploit, leak offset, BOF offset, payload는 포함되어 있지 않다.

## 빌드

```bash
unzip day079_menu_uaf_bof.zip
cd day079_menu_uaf_bof
chmod +x run.sh
./run.sh
```

직접 실행:

```bash
./day79_menu
```

## 메뉴

```text
1. add
2. free
3. show
4. bof
5. exit
```

- `add`: 원하는 slot과 크기로 heap chunk 할당
- `free`: chunk를 해제하지만 slot 포인터를 지우지 않음
- `show`: 해당 slot 주소에서 raw 16바이트 출력
- `bof`: 별도의 stack buffer overflow 입력

## 목표

1. 큰 chunk와 guard chunk의 배치를 직접 구성한다.
2. 큰 chunk를 free해 unsorted bin에 들어가도록 만든다.
3. dangling pointer가 남은 slot을 `show`하여 raw `fd`/`bk`를 확인한다.
4. leak을 파싱하고 현재 glibc 기준 offset을 구한다.
5. libc base를 계산하고 GDB mapping과 대조한다.
6. `bof`의 saved RIP offset을 직접 구한다.
7. gadget, `system`, `"/bin/sh"` 주소를 계산한다.
8. ROP로 shell을 얻고 명령 실행까지 확인한다.

## 성공 조건

- 자동 출력이 아니라 메뉴 조작으로 leak 조건을 직접 만들었음
- victim이 top chunk와 병합되지 않도록 guard를 사용했음
- raw 16바이트에서 fd와 bk를 각각 확인했음
- 계산한 libc base가 offset-0 libc mapping과 일치함
- `system` 주소와 `"/bin/sh"` 주소를 독립적으로 검증함
- saved RIP offset을 cyclic과 disassembly로 교차검증함
- stack alignment를 확인함
- shell에서 실제 명령 실행에 성공함

## 실패 케이스도 확인

- guard 없이 victim을 top과 인접하게 둔 경우
- 작은 chunk를 free해 tcache로 들어간 경우
- free 전에 show한 경우
- 잘못된 libc leak offset을 사용한 경우
- stack alignment가 틀린 경우

## 보호기법

예상 구성:

```text
Canary: OFF
NX: ON
PIE: OFF
ASLR: 시스템 설정에 따라 ON
```
