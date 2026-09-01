#include <stdint.h>
#include <unistd.h>

int main(void)
{
    uint8_t n = 0;
    volatile unsigned int sum = 0;

    if (read(STDIN_FILENO, &n, 1) != 1)
        return 0;

    for (unsigned int i = 0; i < n; i++)
        sum += i;

    return sum == 0xffffffff;
}
