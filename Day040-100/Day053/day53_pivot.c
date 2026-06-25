#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

char fake_stack[0x300];

void setup() {
    setvbuf(stdin,  NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
}

void win() {
    puts("[+] WIN: stack pivot success");
    system("/bin/sh");
}

void vuln() {
    char buf[64];

    puts("stage1: write fake stack");
    read(0, fake_stack, 0x300);

    puts("stage2: pivot trigger");
    read(0, buf, 88);

    puts("bye");
}

int main() {
    setup();
    vuln();
    return 0;
}
