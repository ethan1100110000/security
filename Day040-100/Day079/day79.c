#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

__attribute__((naked, used))
void pop_rdi_ret(void)
{
    __asm__("pop %rdi; ret");
}

__attribute__((noinline))
static void vuln(void)
{
    char buf[64];

    puts("INPUT:");
    read(STDIN_FILENO, buf, 300);
}

int main(void)
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    void *victim = malloc(0x500);
    void *guard  = malloc(0x20);

    if (victim == NULL || guard == NULL) {
        perror("malloc");
        return 1;
    }

    free(victim);

    puts("LEAK:");
    if (write(STDOUT_FILENO, victim, 8) != 8) {
        perror("write");
        return 1;
    }
    putchar('\n');

    vuln();

    free(guard);
    return 0;
}
