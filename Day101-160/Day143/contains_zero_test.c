#include <stddef.h>
#include <stdio.h>
int contains_zero(const unsigned char *data, size_t n);
int main(void)
{
    const unsigned char data[] = {0x41, 0x00, 0x42};
    const size_t lengths[] = {0, 1, 3};
    const int expected[] = {0, 0, 1};
    int failed = 0;
    for (size_t k = 0; k < 3; ++k) {
        int result = contains_zero(data, lengths[k]);
        printf("n=%zu result=%d expected=%d %s\n", lengths[k], result,
               expected[k], result == expected[k] ? "PASS" : "FAIL");
        if (result != expected[k]) failed = 1;
    }
    const unsigned char no_zero[] = {0x41, 0x42, 0x43};
    int result = contains_zero(no_zero, sizeof(no_zero));
    printf("no_zero n=3 result=%d expected=0 %s\n", result, result == 0 ? "PASS" : "FAIL");
    if (result != 0) failed = 1;
    return failed;
}
