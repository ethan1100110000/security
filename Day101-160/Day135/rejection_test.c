#include <stdio.h>
#include <string.h>

typedef int (*copy_fn)(unsigned char *, size_t, const unsigned char *, size_t, size_t);

static int original(unsigned char *dst, size_t capacity,
                    const unsigned char *src, size_t src_size, size_t length)
{
    if (length > src_size) return 1;
    if (length > capacity) return 2;
    memcpy(dst, src, length);
    return 0;
}

static int reversed(unsigned char *dst, size_t capacity,
                    const unsigned char *src, size_t src_size, size_t length)
{
    if (length > capacity) return 2;
    if (length > src_size) return 1;
    memcpy(dst, src, length);
    return 0;
}

/* Deliberate contract violation: this example has valid first-byte access. */
static int early_write(unsigned char *dst, size_t capacity,
                       const unsigned char *src, size_t src_size, size_t length)
{
    dst[0] = src[0];
    if (length > src_size) return 1;
    if (length > capacity) return 2;
    memcpy(dst, src, length);
    return 0;
}

static int check(const char *name, copy_fn fn, int expected_rc,
                 int expected_unchanged)
{
    const unsigned char src[4] = {0x41, 0x42, 0x43, 0x44};
    unsigned char dst[16];
    unsigned char before[16];
    memset(dst, 0xAA, sizeof(dst));
    memcpy(before, dst, sizeof(dst));
    int rc = fn(dst, sizeof(dst), src, sizeof(src), 17);
    int rc_ok = rc == expected_rc;
    int unchanged = memcmp(dst, before, sizeof(dst)) == 0;
    int expected_behavior = rc_ok && unchanged == expected_unchanged;
    printf("%s src_size=4 capacity=16 length=17 rc=%d expected_rc=%d "
           "return_check=%s unchanged_check=%s first_byte=%02X "
           "contract=%s expected_behavior=%s\n",
           name, rc, expected_rc, rc_ok ? "PASS" : "FAIL",
           unchanged ? "PASS" : "FAIL", (unsigned int)dst[0],
           rc_ok && unchanged ? "PASS" : "FAIL",
           expected_behavior ? "PASS" : "FAIL");
    return expected_behavior ? 0 : 1;
}

int main(void)
{
    int failures = 0;
    failures += check("original", original, 1, 1);
    failures += check("reversed", reversed, 2, 1);
    failures += check("early_write", early_write, 1, 0);
    printf("summary: %d/3 expected behaviors observed; "
           "early_write contract failure is intentional\n", 3 - failures);
    return failures == 0 ? 0 : 1;
}
