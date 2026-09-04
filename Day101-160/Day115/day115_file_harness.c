#include "day115_parser.h"

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s input_file\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);

    if (fd < 0) {
        perror("open");
        return 1;
    }

    uint8_t input[256];
    ssize_t received = read(fd, input, sizeof(input));

    if (received < 0) {
        perror("read");
        close(fd);
        return 1;
    }

    close(fd);

    return parse_record(input, (size_t)received) == 0 ? 0 : 1;
}
