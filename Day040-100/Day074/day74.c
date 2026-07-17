#include <stdio.h>
#include <stdlib.h>

struct Holder {
    char padding[0x20];
    void *ptr;
};

int main(void)
{
    setbuf(stdout, NULL);

    char *a = malloc(0x30);
    struct Holder *holder = malloc(sizeof(struct Holder));
    char *test = malloc(0x30);
    char *target = malloc(0x30);

    if (!a || !holder || !target)
        return 1;

    holder->ptr = target;

    printf("holder = %p\n", (void *)holder);
    printf("leak   = %p\n", holder->ptr);

    getchar();

    free(target);
    free(holder);
    free(a);
    return 0;
}
