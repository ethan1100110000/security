#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void pwnme(void);

void setup(void) {
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
}

void win(void) {
    puts("Good. Canary and PIE both bypassed.");
    system("/bin/cat flag.txt");
}

void usefulGadgets(void) {
    __asm__("ret;");
}

void banner(void) {
    puts("=== Day40 Canary + PIE + BOF ===");
    puts("Goal: leak canary and PIE, then control RIP without stack smashing.");
}

int main(void) {
    setup();
    banner();
    pwnme();
    puts("Returned to main safely.");
    return 0;
}
