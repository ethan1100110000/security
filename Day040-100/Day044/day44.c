#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void win() {
    puts("WIN");
    system("/bin/sh");
}

void vuln() {
    char name[96];
    char buf[80];

    puts("Day44 Canary + PIE");
    printf("name: ");
    fflush(stdout);

    read(0, name, 95);

    printf("hello, ");
    printf(name);
    puts("");

    printf("payload: ");
    fflush(stdout);

    read(0, buf, 220);
}

int main() {

setvbuf(stdout, NULL, _IONBF, 0);
vuln();
return 0;
}
