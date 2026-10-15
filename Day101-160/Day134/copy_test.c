#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int copy_data(char *dst, size_t capacity, const char *src,
              size_t src_size, size_t length)
{
    if (length > src_size) return 1;
    if (length > capacity) return 2;
    memcpy(dst, src, length);
    return 0;
}

static int run_case(const char *name, size_t src_size, size_t length,
                    int expected_rc, int embedded_nul)
{
    unsigned char *src = malloc(src_size);
    unsigned char dst[16];
    unsigned char initial[16];
    if (src == NULL) return 1;
    for (size_t i = 0; i < src_size; ++i)
        src[i] = (unsigned char)(0x41 + i % 26);
    if (embedded_nul) {
        const unsigned char binary_input[4] = {0x41, 0x00, 0x42, 0x43};
        memcpy(src, binary_input, sizeof(binary_input));
    }
    memset(dst, 0xAA, sizeof(dst));
    memcpy(initial, dst, sizeof(dst));

    int rc = copy_data((char *)dst, sizeof(dst), (const char *)src,
                       src_size, length);
    int content_ok = 1;
    int unchanged_ok;
    if (expected_rc == 0) {
        content_ok = memcmp(dst, src, length) == 0;
        unchanged_ok = memcmp(dst + length, initial + length,
                              sizeof(dst) - length) == 0;
    } else {
        unchanged_ok = memcmp(dst, initial, sizeof(dst)) == 0;
    }
    int pass = rc == expected_rc && content_ok && unchanged_ok;
    printf("%s src_size=%zu capacity=%zu length=%zu rc=%d expected=%d "
           "content=%s unchanged=%s result=%s dst=",
           name, src_size, sizeof(dst), length, rc, expected_rc,
           expected_rc == 0 ? (content_ok ? "PASS" : "FAIL") : "N/A",
           unchanged_ok ? "PASS" : "FAIL", pass ? "PASS" : "FAIL");
    for (size_t i = 0; i < sizeof(dst); ++i)
        printf("%s%02X", i == 0 ? "" : " ", (unsigned int)dst[i]);
    putchar('\n');
    free(src);
    return pass ? 0 : 1;
}

int main(void)
{
    int failures = 0;
    failures += run_case("source_equal", 4, 4, 0, 0);
    failures += run_case("source_plus_one", 4, 5, 1, 0);
    failures += run_case("destination_equal", 32, 16, 0, 0);
    failures += run_case("destination_plus_one", 32, 17, 2, 0);
    failures += run_case("embedded_nul", 4, 4, 0, 1);
    printf("summary: %d/5 passed\n", 5 - failures);
    return failures == 0 ? 0 : 1;
}
