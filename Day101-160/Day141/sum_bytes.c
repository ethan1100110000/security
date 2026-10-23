#include <stddef.h>
unsigned int sum_bytes(const unsigned char *data, size_t n) {
    unsigned int sum = 0;
    for (size_t i = 0; i < n; ++i) sum += data[i];
    return sum;
}
