CC=gcc
CFLAGS=-fno-stack-protector -no-pie -z noexecstack -fcf-protection=none -O0 -g

all: day54_pivot

day54_pivot: day54_pivot.c
	$(CC) day54_pivot.c -o day54_pivot $(CFLAGS)

clean:
	rm -f day54_pivot
