/*
 * Day154 controlled protection experiment (x86-64 Linux, local only).
 * The three modes isolate distinct memory-security questions:
 *   data: adjacent field modification within a stack struct
 *   overflow: overlong write into a stack char array
 *   execute: call a harmless, one-byte RET stub from the stack
 * Compare 2x2 combinations of stack canaries and stack NX.
 */
#define _POSIX_C_SOURCE 200809L
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct frame_data {
    unsigned char bytes[16];
    uint64_t marker;
};

__attribute__((noinline))
static void data_probe(void) {
    struct frame_data frame;
    memset(&frame, 0, sizeof(frame));
    frame.marker = UINT64_C(0x1122334455667788);
    puts("[data] enter 20 bytes");
    /* Deliberate bytes-member overrun that stays inside the enclosing struct. */
    volatile size_t requested = 20;
    ssize_t n = read(STDIN_FILENO, frame.bytes, requested);
    printf("[data] read=%zd marker=0x%016" PRIx64 "\n", n, frame.marker);
}

__attribute__((noinline))
static void overflow_probe(void) {
    unsigned char local[32];
    memset(local, 0, sizeof(local));
    puts("[overflow] enter up to 80 bytes");
    /* Intentionally unsafe local lab target: 80 may exceed 32. */
    volatile size_t requested = 80;
    ssize_t n = read(STDIN_FILENO, local, requested);
    printf("[overflow] read=%zd first=0x%02x\n", n, (unsigned)local[0]);
}

__attribute__((noinline))
static void exec_probe(void) {
    /* The one instruction is RET (0xc3). This is not a shellcode payload. */
    volatile unsigned char stack_code[16] = {0xc3};
    void (*fn)(void) = (void (*)(void))(uintptr_t)(const void *)stack_code;
    puts("[execute] calling harmless RET instruction on stack");
    fn();
    puts("[execute] returned from stack instruction");
}

int main(int argc, char **argv) {
    setbuf(stdout, NULL);
    if (argc != 2) {
        fprintf(stderr, "usage: %s data|overflow|execute\n", argv[0]);
        return 2;
    }
    if (strcmp(argv[1], "data") == 0) data_probe();
    else if (strcmp(argv[1], "overflow") == 0) overflow_probe();
    else if (strcmp(argv[1], "execute") == 0) exec_probe();
    else {
        fprintf(stderr, "unknown mode\n");
        return 2;
    }
    return 0;
}
