#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

unsigned char auth = 0x00;

void banner(void) {
    puts("== Day51 FSB Byte Write ==");
    puts("Goal: set auth to 0x7f using one format-string input.");
}

void vuln(void) {
    char buf[256];
    memset(buf, 0, sizeof(buf));

    printf("auth is at: %p\n", &auth);
    puts("say something:");

    read(0, buf, 240);
    printf(buf);
    printf("\nauth now: 0x%02x\n", auth);

    if (auth == 0x7f) {
        puts("ACCESS GRANTED");
        system("/bin/sh");
    } else {
        puts("ACCESS DENIED");
    }
}

int main(void) {
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    banner();
    vuln();
    return 0;
}
