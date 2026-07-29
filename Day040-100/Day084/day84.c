#include <stdio.h>
#include <stdlib.h>

int main(void) {
    char *a = malloc(0x30);
    char *b = malloc(0x30);

    printf("a = %p\n", a);
    printf("b = %p\n", b);

    free(a);
    free(b);

    char *p = malloc(0x30);
    printf("p = %p\n", p);
}
