/*
 * Day153: ASan comparison model for PXN1 parser.
 * Independently reconstructed teaching example, NOT recovered source.
 * In particular, local_buffer[48] is a deliberate hypothesis to test;
 * the original stripped ELF's declared array size has not been proven.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int process(const unsigned char *input, size_t read_count) {
    if (read_count < 8) {
        puts("short record");
        return -1;
    }

    if (memcmp(input, "PXN1", 4) != 0) {
        puts("bad marker");
        return -2;
    }

    uint16_t a = (uint16_t)input[4] | ((uint16_t)input[5] << 8);
    uint16_t b = (uint16_t)input[6] | ((uint16_t)input[7] << 8);

    if (a > 112 || a > read_count - 8) {
        puts("bad length");
        return -3;
    }

    /* 48 bytes is a test hypothesis based on the observed memset length. */
    unsigned char local_buffer[48];
    memset(local_buffer, 0, 48);

    /* ASan should diagnose this for a > 48 in this teaching model. */
    memcpy(local_buffer, input + 8, a);

    unsigned int sum = 0;
    for (size_t i = 0; i < a && i < 48; ++i) {
        sum += local_buffer[i];
    }

    printf("accepted: mode=%u sum=%u\n", (unsigned)b, sum);
    return 0;
}

int main(void) {
    /* 192 bytes chosen for a safe reproduction of read's observed limit;
       this does not assert the original ELF's main array declaration. */
    unsigned char input[192];
    ssize_t n = read(STDIN_FILENO, input, sizeof(input));
    if (n <= 0) {
        puts("no input");
        return 1;
    }
    return process(input, (size_t)n) == 0 ? 0 : 1;
}
