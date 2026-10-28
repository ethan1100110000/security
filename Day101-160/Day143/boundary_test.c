#include <stddef.h>
#include <stdio.h>
int contains_zero(const unsigned char *data, size_t n);
int main(void)
{
    const unsigned char data[5] = {0x41, 0x42, 0x43, 0x00, 0x44};
    const size_t lengths[2] = {3, 4};
    const int expected[2] = {0, 1};
    int failed = 0;
    for (size_t k = 0; k < 2; ++k) {
        int result = contains_zero(data, lengths[k]);
        printf("n=%zu result=%d expected=%d %s\n", lengths[k], result,
               expected[k], result == expected[k] ? "PASS" : "FAIL");
        if (result != expected[k]) failed = 1;
    }
    return failed;
}
