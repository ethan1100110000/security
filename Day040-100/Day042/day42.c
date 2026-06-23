#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void win() {
    puts("WIN");
    system("/bin/sh");
}

void vuln() {
    char name[64];
    char buf[64];

    puts("Day42 Canary + BOF");
    printf("name: ");
    fflush(stdout);

    read(0, name, 63);
    printf("hello, ");
    printf(name);
    puts("");

    printf("payload: ");
    fflush(stdout);

    read(0, buf, 200);
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    vuln();
    return 0;
}
