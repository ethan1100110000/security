#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void win() {
    system("/bin/sh");
}

void vuln() {
    char name[128];

    puts("Day41 FSB leak");
    printf("name: ");
    fflush(stdout);

    read(0, name, 127);

    printf("hello, ");
    printf(name);
    puts("");
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    vuln();
    return 0;
}
