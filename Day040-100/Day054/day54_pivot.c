#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

char fake_stack1[0x500];
char fake_stack2[0x500];
int round_no = 0;

__attribute__((naked, used)) void gadget_pop_rdi() {
    __asm__("pop %rdi; ret");
}

__attribute__((naked, used)) void gadget_ret() {
    __asm__("ret");
}

__attribute__((naked, used)) void gadget_leave_ret() {
    __asm__("leave; ret");
}

void setup() {
    setvbuf(stdin,  NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
}

void vuln() {
    char buf[64];
    char *dst;

    if (round_no == 0) {
        dst = fake_stack1;
        round_no = 1;
        puts("stage1: write fake stack 1");
    } else {
        dst = fake_stack2;
        puts("stage1: write fake stack 2");
    }

    read(0, dst, 0x500);

    puts("stage2: pivot trigger");
    read(0, buf, 96);

    puts("bye");
}

int main() {
    setup();
    vuln();
    return 0;
}
