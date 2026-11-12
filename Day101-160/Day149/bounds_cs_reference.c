#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* Reference source reconstructed from the instructor-provided Day149
   supplementary task. The user's local bounds_cs.c was not uploaded;
   the user reported compiling/running this example and its outputs. */
int process(size_t input_size, int32_t len)
{
    unsigned char input[20] = {0};
    unsigned char payload[8] = {0};

    if (input_size > sizeof(input))
        return -5;

    if (input_size < 4)
        return -1;

    if (len < 0)
        return -2;

    if ((size_t)len > input_size - 4)
        return -3;

    if ((size_t)len > sizeof(payload))
        return -4;

    memcpy(payload, input + 4, (size_t)len);

    return 0;
}

int main(void)
{
    printf("A = %d\n", process(12, 8));
    printf("B = %d\n", process(12, 9));
    printf("C = %d\n", process(20, 9));
    printf("D = %d\n", process(20, -1));

    return 0;
}
