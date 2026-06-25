#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void setup(void) {
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
}

__attribute__((naked, used))
void gadget_farm(void) {
    __asm__(
        "pop %rdi; ret;"
        "pop %rsi; ret;"
        "pop %rdx; ret;"
        "leave; ret;"
    );
}

void win(void) {
    puts("[+] ADVANCED WIN: staged read + stack pivot success");
    __asm__ volatile("xor %%rdi, %%rdi; mov $60, %%rax; syscall" ::: "rax", "rdi");
}

void vuln(void) {
    char buf[64];
    puts("hard pivot input:");
    read(0, buf, 0x90);
    puts("bye");
}

int main(void) {
    setup();
    vuln();
    return 0;
}
