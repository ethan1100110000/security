#include <stdio.h>
#include <unistd.h>

void pwnme(void) {
    char fmt[128];
    char buf[64];

    puts("stage1: leak?");
    read(0, fmt, 127);
    printf(fmt);

    puts("\nstage2: overflow?");
    read(0, buf, 220);

    puts("Bye!");
}
