#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main() {
    char *a = malloc(0x30);
    char *b = malloc(0x30);
    char *c = malloc(0x30);

    printf("a = %p\n", a);
    printf("b = %p\n", b);
    printf("c = %p\n", c);

    free(a);
    free(b);
    free(c);

    puts("after free(a), free(b), free(c)");
    getchar();

    char *d = malloc(0x30);
    printf("d = %p\n", d);

    puts("after d = malloc(0x30)");
    getchar();

    return 0;
}
