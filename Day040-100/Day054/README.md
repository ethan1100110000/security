# Day54 Pivot + ret2libc Lab

## Build / Run

```bash
tar -xzf day54_pivot_ret2libc_wsl.tar.gz
cd day54_pivot_ret2libc_wsl
make clean && make
./check.sh
./run.sh
```

## Goal

- stage1-1: write leak ROP chain into `.bss` `fake_stack1`
- stage2-1: overwrite saved `rbp` and saved `rip`, pivot with `leave; ret`
- leak libc address with `puts(puts@got)` and return to `main`
- stage1-2: write final ROP chain into `.bss` `fake_stack2`
- stage2-2: pivot again and call `system("/bin/sh")`

## Expected offset

- `buf[64]`
- saved `rbp` starts after 80 bytes
- saved `rip` starts after 88 bytes
- pivot payload shape: `b"A"*80 + p64(fake) + p64(leave_ret)`
