#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    uint32_t value = UINT32_C(0x12345678);
    const unsigned char *bytes = (const unsigned char *)&value;
    const unsigned char input[2] = {0x00, 0x10};
    uint16_t big = (uint16_t)(((uint32_t)input[0] << 8) | input[1]);
    uint16_t little = (uint16_t)(((uint32_t)input[1] << 8) | input[0]);

    printf("value=0x%08" PRIX32 "\n", value);
    printf("memory low-to-high:");
    for (size_t i = 0; i < sizeof(value); ++i)
        printf(" %02X", (unsigned int)bytes[i]);
    putchar('\n');
    printf("input: %02X %02X\n", (unsigned int)input[0],
           (unsigned int)input[1]);
    printf("big-endian decode: 0x%04X (%u)\n", (unsigned int)big,
           (unsigned int)big);
    printf("little-endian decode: 0x%04X (%u)\n", (unsigned int)little,
           (unsigned int)little);
    return 0;
}
