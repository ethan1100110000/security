// day48_shellcode.c
#include <stdio.h>
#include <unistd.h>

void vuln() {
    char buf[128];

    printf("buf is at: %p\n", buf);
    puts("input:");

    read(0, buf, 300);

    puts("bye");
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    vuln();
    return 0;
}
