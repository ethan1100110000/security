#include <stdio.h>
extern int lesson_helper(void);
int main(int argc, char **argv)
{
    (void)argv;
    puts("main entered");
    fflush(stdout);
    if (argc > 1)
        printf("helper result=%d\n", lesson_helper());
    else
        puts("helper call skipped");
    return 0;
}
