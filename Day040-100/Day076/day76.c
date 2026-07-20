#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    void *a = malloc(0x500);
    //void *guard = malloc(0x20);

    memset(a, 'A', 0x20);

    printf("a     = %p\n", a);
    printf("guard = %p\n", guard);

    puts("[before free]");
    getchar();

    free(a);

    puts("[after free]");
    getchar();

    free(guard);
    return 0;
}
