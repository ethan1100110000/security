CC := gcc
CFLAGS := -O0 -g -Wall -Wextra -fno-stack-protector -fcf-protection=none -no-pie
LDFLAGS := -Wl,-z,noexecstack -no-pie

all: day79

day79: day79.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

check: day79
	checksec --file=./day79 || true

clean:
	rm -f day79 core core.*

.PHONY: all check clean
