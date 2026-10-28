#include <stddef.h>
int contains_zero(const unsigned char *data, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        if (data[i] == 0)
            return 1;
    }
    return 0;
}
