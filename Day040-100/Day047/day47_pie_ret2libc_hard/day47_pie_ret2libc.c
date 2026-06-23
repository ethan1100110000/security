// Day47 local challenge: PIE leak + libc leak + ret2libc
// Build: gcc day47_pie_ret2libc_hard.c -o day47_pie_ret2libc_hard -fno-stack-protector -fPIE -pie -O0 -Wl,-z,relro -Wl,-z,lazy
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

char hotel_note[0x80];

__attribute__((naked, used)) void pop_rdi_ret(void) {
    __asm__("pop %rdi; ret;");
}

__attribute__((naked, used)) void align_ret(void) {
    __asm__("ret;");
}

__attribute__((naked, used)) void pop_rsi_r15_ret(void) {
    __asm__("pop %rsi; pop %r15; ret;");
}

__attribute__((naked, used)) void pop_rdx_ret(void) {
    __asm__("pop %rdx; ret;");
}

static void banner(void) {
    puts("===============================");
    puts("   DAY47 HARD HOTEL - LOCAL");
    puts("===============================");
}

static void leak_lobby(void) {
    char fmt[0x90];
    memset(fmt, 0, sizeof(fmt));
    puts("[1] Leave a short review:");
    ssize_t n = read(0, fmt, sizeof(fmt) - 1);
    if (n <= 0) {
        puts("bye");
        exit(0);
    }
    printf("guest says: ");
    printf(fmt);
    puts("");
}

static void rop_lobby(void) {
    char buf[0x70];
    memset(buf, 0, sizeof(buf));
    puts("[2] Any final request?");
    read(0, buf, 0x220);
    puts("request noted");
}

int main(void) {
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    banner();
    strcpy(hotel_note, "no free leaks today");
    leak_lobby();
    rop_lobby();
    puts("checkout complete");
    return 0;
}
