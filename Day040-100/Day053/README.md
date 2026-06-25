# Day053 Stack Pivot Lab - WSL Ready

## 바로 실행

```bash
tar -xzf day53_pivot_wsl.tar.gz
cd day53_pivot_wsl
./check.sh
./run.sh
```

## 재컴파일

```bash
make clean
make
```

## 목표

- stage1: `.bss`의 `fake_stack`에 fake ROP chain 작성
- stage2: stack overflow로 saved `rbp`, saved `rip` 덮기
- `saved rbp = fake_stack`
- `saved rip = leave; ret`
- gdb에서 `rsp`가 `.bss`로 pivot되는지 확인
