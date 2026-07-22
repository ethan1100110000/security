CC := gcc
CFLAGS := -O0 -g -Wall -Wextra -fno-stack-protector -fcf-protection=none -no-pie
LDFLAGS := -Wl,-z,noexecstack -no-pie

all: day79_menu

day79_menu: day79_menu.c
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

check: day79_menu
	checksec --file=./day79_menu || true

clean:
	rm -f day79_menu core core.*

.PHONY: all check clean
