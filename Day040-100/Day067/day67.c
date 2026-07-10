#include <stdio.h>
#include <stdlib.h>

int main() {
    char *a = malloc(0x30);

    printf("a = %p\n", a);

    free(a);
    puts("first free done");

    free(a);
    puts("second free done");

    return 0;
}
