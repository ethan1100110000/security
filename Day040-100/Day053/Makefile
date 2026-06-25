CC=gcc
CFLAGS=-fno-stack-protector -no-pie -z noexecstack -fcf-protection=none -O0

all: day53_pivot

day53_pivot: day53_pivot.c
	$(CC) day53_pivot.c -o day53_pivot $(CFLAGS)

clean:
	rm -f day53_pivot
