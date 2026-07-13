#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    setbuf(stdout, NULL);

    char *a = malloc(0x30);
    char *b = malloc(0x30);

    if (a == NULL || b == NULL) {
        perror("malloc");
        return 1;
    }

    printf("[+] a = %p\n", (void *)a);
    printf("[+] b = %p\n", (void *)b);

    /*
     * tcache[0x40]:
     * free(a) -> a
     * free(b) -> b -> a
     */
    free(a);
    free(b);

    /*
     * 해제된 a의 tcache key를 UAF write로 손상시킨다.
     * glibc 버전에 따라 동작이 달라질 수 있다.
     */
    ((uintptr_t *)a)[1] = 0;

    /*
     * a를 다시 free해서:
     * tcache[0x40] -> a -> b -> a
     */
    free(a);

    char *p = malloc(0x30);
    char *q = malloc(0x30);
    char *r = malloc(0x39);

    printf("\n[+] p = %p\n", (void *)p);
    printf("[+] q = %p\n", (void *)q);
    printf("[+] r = %p\n", (void *)r);

    printf("\n[+] p == r: %s\n", p == r ? "true" : "false");
    printf("[+] p == q: %s\n", p == q ? "true" : "false");

    /*
     * q는 별도 chunk라는 비교 기준을 만든다.
     */
    memset(q, 'Q', 0x30);

    /*
     * p와 r이 같은 chunk라면 p를 통한 write가 r에서도 보인다.
     */
    memset(p, 'A', 0x30);

    printf("\n[+] p data: %.8s\n", p);
    printf("[+] r data: %.8s\n", r);
    printf("[+] q data: %.8s\n", q);

    return 0;
}
