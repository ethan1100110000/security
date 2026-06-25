CC=gcc
CFLAGS=-fno-stack-protector -no-pie -z noexecstack -fcf-protection=none -O0

all: day53_pivot_hard

day53_pivot_hard: day53_pivot_hard.c
	$(CC) $(CFLAGS) $< -o $@
	strip -s $@

clean:
	rm -f day53_pivot_hard
