#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

__attribute__((noinline, used))
void archive_open(void)
{
    puts("archive: local training complete");
}

__attribute__((noinline))
static ssize_t receive_bytes(void *dst, size_t count)
{
    return read(STDIN_FILENO, dst, count);
}

__attribute__((noinline))
static void preview(void)
{
    char label[48];
    fputs("label> ", stdout);
    if (!fgets(label, sizeof(label), stdin))
        return;
    fputs("preview: ", stdout);
    printf(label);
}

__attribute__((noinline))
static void submit(void)
{
    char record[64];
    fputs("record> ", stdout);
    ssize_t n = receive_bytes(record, 112);
    if (n < 0) {
        perror("read");
        return;
    }
    printf("received: %zd\n", n);
}

int main(void)
{
    char command[16];
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    puts("Archive desk / local exercise");
    for (;;) {
        fputs("1 preview / 2 submit / 3 quit\n> ", stdout);
        if (!fgets(command, sizeof(command), stdin))
            break;
        switch (strtol(command, NULL, 10)) {
        case 1: preview(); break;
        case 2: submit(); break;
        case 3: return 0;
        default: puts("unknown choice"); break;
        }
    }
    return 0;
}
