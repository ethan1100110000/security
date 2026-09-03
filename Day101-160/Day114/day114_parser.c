#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

enum parser_state {
    ST_START,
    ST_MAGIC,
    ST_VERSION,
    ST_COMMAND,
    ST_LENGTH,
    ST_PAYLOAD,
    ST_DONE
};

static int reject(enum parser_state state, const char *reason)
{
    fprintf(stderr, "reject at state %d: %s\n", state, reason);
    return -1;
}

static int parse_record(const uint8_t *input, size_t input_size)
{
    enum parser_state state = ST_START;

    if (input_size < 7)
        return reject(state, "short header");

    state = ST_MAGIC;

    if (input[0] != 'T')
        return reject(state, "bad magic[0]");
    if (input[1] != 'P')
        return reject(state, "bad magic[1]");
    if (input[2] != 'A')
        return reject(state, "bad magic[2]");
    if (input[3] != 'R')
        return reject(state, "bad magic[3]");

    state = ST_VERSION;

    if (input[4] != 1)
        return reject(state, "unsupported version");

    state = ST_COMMAND;

    if (input[5] != 1 && input[5] != 2)
        return reject(state, "unknown command");

    state = ST_LENGTH;

    uint8_t length = input[6];

    if ((size_t)length > input_size - 7)
        return reject(state, "truncated payload");

    state = ST_PAYLOAD;

    char payload[16] = {0};

    /* 의도적 취약점: length와 sizeof(payload)를 비교하지 않는다. */
    memcpy(payload, input + 7, length);

    if (input[5] == 1) {
        size_t output_length = length;

        if (output_length > sizeof(payload))
            output_length = sizeof(payload);

        fwrite(payload, 1, output_length, stdout);
        putchar('\n');
    } else {
        unsigned int checksum = 0;
        size_t count = length;

        if (count > sizeof(payload))
            count = sizeof(payload);

        for (size_t i = 0; i < count; i++)
            checksum += (unsigned char)payload[i];

        printf("checksum=%u\n", checksum);
    }

    state = ST_DONE;
    printf("state=%d\n", state);
    return 0;
}

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
