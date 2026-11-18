// Day155 — benign, local ASLR/PIE address probe.
// Prints controlled addresses; no overflow or exploitation logic.
#define _GNU_SOURCE
#include <dlfcn.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

__attribute__((noinline))
static int helper(int x) { return x + 1; }

int main(void) {
    volatile unsigned char stack_local[64] = {0};
    void *libc_puts = dlsym(RTLD_DEFAULT, "puts");
    // Linux x86-64 function addresses represented as uintptr_t for this lab.
    uintptr_t main_addr = (uintptr_t)&main;
    uintptr_t helper_addr = (uintptr_t)&helper;
    uintptr_t offset = main_addr - helper_addr;

    printf("main=0x%" PRIxPTR " helper=0x%" PRIxPTR
           " stack=0x%" PRIxPTR " libc_puts=0x%" PRIxPTR
           " main_minus_helper=0x%" PRIxPTR "\n",
           main_addr, helper_addr, (uintptr_t)&stack_local[0],
           (uintptr_t)libc_puts, offset);
    return helper((int)stack_local[0]) - 1;
}
