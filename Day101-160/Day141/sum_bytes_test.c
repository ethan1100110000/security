#include <stddef.h>
#include <stdio.h>
unsigned int sum_bytes(const unsigned char *, size_t);
int main(void) {
    const unsigned char data[] = {1, 0, 3};
    const size_t lengths[] = {0, 1, 3};
    const unsigned int expected[] = {0, 1, 4};
    int failed = 0;
    for (size_t k = 0; k < 3; ++k) {
        unsigned int result = sum_bytes(data, lengths[k]);
        printf("n=%zu result=%u expected=%u %s\n", lengths[k], result,
               expected[k], result == expected[k] ? "PASS" : "FAIL");
        if (result != expected[k]) failed = 1;
    }
    return failed;
}
