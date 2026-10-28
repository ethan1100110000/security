#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
int contains_zero(const unsigned char *data, size_t n);
int main(void)
{
    unsigned char buffer[16] = {0};
    ssize_t received = read(STDIN_FILENO, buffer, sizeof(buffer));
    if (received < 0) { perror("read"); return 1; }
    if (received != 4) { fprintf(stderr, "test requires exactly 4 input bytes\n"); return 1; }
    size_t input_size = (size_t)received;
    int input_result = contains_zero(buffer, input_size);
    int oversized_input_result = contains_zero(buffer, 8);
    printf("capacity=%zu received=%zu\n", sizeof(buffer), input_size);
    printf("n=received result=%d expected=0\n", input_result);
    printf("n=8 result=%d expected=1 (zero beyond input)\n", oversized_input_result);
    size_t requested = 8;
    int rejected = requested > input_size;
    printf("caller requested=8 rejected=%s\n", rejected ? "YES" : "NO");
    return !(input_result == 0 && oversized_input_result == 1 && rejected);
}
