#include <stdio.h>
#include <string.h>

int copy_data(char *dst, size_t capacity, const char *src,
              size_t src_size, size_t length)
{
    if (length > src_size) return 1;
    if (length > capacity) return 2;
    memcpy(dst, src, length);
    return 0;
}

int main(void)
{
    const char src[4] = {0x41, 0x00, 0x42, 0x43};
    unsigned char dst[16];
    unsigned char before[16];
    memset(dst, 0xAA, sizeof(dst));
    memcpy(before, dst, sizeof(dst));
    int rc = copy_data((char *)dst, sizeof(dst), src, sizeof(src), 0);
    int unchanged = memcmp(dst, before, sizeof(dst)) == 0;
    printf("src_size=4 capacity=16 length=0 rc=%d expected_rc=0 unchanged=%s dst=",
           rc, unchanged ? "PASS" : "FAIL");
    for (size_t i = 0; i < sizeof(dst); ++i)
        printf("%s%02X", i ? " " : "", (unsigned int)dst[i]);
    putchar('\n');
    return rc == 0 && unchanged ? 0 : 1;
}
