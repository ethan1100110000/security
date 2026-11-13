#include <stdio.h>

__attribute__((noinline))
int inspect(int a, int b, int c, int d, int e, int f)
{
    volatile int local[4] = {a, b, c, d};
    return local[0] + local[1] + local[2] + local[3] + e + f;
}

int main(void)
{
    int result = inspect(11, 22, 33, 44, 55, 66);
    printf("result=%d\n", result);
    return 0;
}
