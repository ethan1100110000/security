/* Reconstructed teaching model, not the original build source. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>

struct record {
    char label[25];
    uint8_t category;
    uint8_t reserved[7];
};

static int parse_record(const char *input)
{
    size_t total = strlen(input);
    if (total <= 2 || input[0] < '1' || input[0] > '3' || input[1] != ':') {
        puts("invalid record");
        return 1;
    }
    size_t len = total - 2;
    if (len > 25) {
        puts("record too long");
        return 2;
    }
    struct record rec = {0};
    if (sizeof(rec.label) <= len) {
        puts("overflow");
        return 4;
    }
    rec.category = (uint8_t)(input[0] - '0');
    memcpy(rec.label, input + 2, len);
    rec.label[len] = '\0';
    printf("category=%u size=%zu label=%.*s\n",
           (unsigned)rec.category, len, (int)len, rec.label);
    return 0;
}

int main(void)
{
    char input[128];
    puts("Enter a record:");
    if (!fgets(input, sizeof(input), stdin))
        return 3;
    input[strcspn(input, "\n")] = '\0';
    return parse_record(input);
}
