CC := gcc
CFLAGS := -O0 -g -fno-stack-protector -no-pie -fcf-protection=none
LDFLAGS := -Wl,-z,relro,-z,now

all: day83

day83: day83.c
	$(CC) $(CFLAGS) day83.c -o day83 $(LDFLAGS)

clean:
	rm -f day83

.PHONY: all clean
