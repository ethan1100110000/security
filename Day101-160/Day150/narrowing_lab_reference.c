#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

/* Day150 instructor reference for narrowing and array-bound checks.
   Reconstructed from provided teaching code. User's original local
   narrowing_lab.c bytes were not uploaded. No unsafe copy is performed. */
int wrong_check(size_t input_size, int32_t len)
{
    uint8_t n = (uint8_t)len;

    if (input_size < 4)
        return -1;

    if (n > 8)
        return -4;

    if (n > input_size - 4)
        return -3;

    return 0;
}

int right_check(size_t input_size, int32_t len)
{
    if (input_size < 4)
        return -1;

    if (len < 0)
        return -2;

    if ((size_t)len > input_size - 4)
        return -3;

    if ((size_t)len > 8)
        return -4;

    return 0;
}

int main(void)
{
    printf("A: wrong=%d right=%d\n",
           wrong_check(12, 8), right_check(12, 8));

    printf("B: wrong=%d right=%d\n",
           wrong_check(12, 256), right_check(12, 256));

    printf("C: wrong=%d right=%d\n",
           wrong_check(12, 9), right_check(12, 9));

    return 0;
}
