#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    void *victim = malloc(0x500);
    void *guard  = malloc(0x20);

    if (victim == NULL || guard == NULL) {
        perror("malloc");
        return 1;
    }

    memset(victim, 'A', 0x20);

    printf("victim = %p\n", victim);
    printf("guard  = %p\n", guard);

    puts("[before free]");
    getchar();

    free(victim);

    puts("[after free]");
    getchar();

    free(guard);
    return 0;
}
