all:
	gcc day47_pie_ret2libc_hard.c -o day47_pie_ret2libc_hard -fno-stack-protector -fPIE -pie -O0 -Wl,-z,relro -Wl,-z,lazy

clean:
	rm -f day47_pie_ret2libc_hard
