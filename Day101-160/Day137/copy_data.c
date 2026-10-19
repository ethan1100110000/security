#include <stddef.h>
#include <string.h>

int copy_data(char *dst, size_t capacity, const char *src,
              size_t src_size, size_t length)
{
    if (length > src_size) return 1;
    if (length > capacity) return 2;
    memcpy(dst, src, length);
    return 0;
}
