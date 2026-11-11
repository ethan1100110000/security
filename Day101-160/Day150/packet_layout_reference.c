#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

/* Assistant-reconstructed reference source for the user's Day150 layout test.
   User reported running matching packet_layout.c locally; exact original file
   bytes from that local machine were not uploaded. */
struct Packet {
    uint8_t type;
    uint32_t length;
    uint8_t flags;
};

int main(void)
{
    printf("sizeof: %zu\n", sizeof(struct Packet));
    printf("type:   %zu\n", offsetof(struct Packet, type));
    printf("length: %zu\n", offsetof(struct Packet, length));
    printf("flags:  %zu\n", offsetof(struct Packet, flags));
    return 0;
}
