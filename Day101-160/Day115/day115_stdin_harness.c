#include "day115_parser.h"

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    uint8_t input[256];
    ssize_t received = read(STDIN_FILENO, input, sizeof(input));

    if (received < 0) {
        perror("read");
        return 1;
    }

    return parse_record(input, (size_t)received) == 0 ? 0 : 1;
}
