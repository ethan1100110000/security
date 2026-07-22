#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SLOT_COUNT 8
#define SHOW_BYTES 16
#define MAX_CHUNK_SIZE 0x1000

static void *chunks[SLOT_COUNT];
static size_t chunk_sizes[SLOT_COUNT];

static unsigned long read_ulong(const char *prompt)
{
    char buf[64];
    char *end = NULL;
    unsigned long value;

    fputs(prompt, stdout);

    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        puts("input error");
        exit(1);
    }

    errno = 0;
    value = strtoul(buf, &end, 0);

    if (errno != 0 || end == buf) {
        puts("invalid number");
        return (unsigned long)-1;
    }

    return value;
}

static int valid_index(unsigned long index)
{
    return index < SLOT_COUNT;
}

static void add_chunk(void)
{
    unsigned long index = read_ulong("index: ");
    unsigned long size = read_ulong("size: ");

    if (!valid_index(index)) {
        puts("invalid index");
        return;
    }

    if (chunks[index] != NULL) {
        puts("slot already used");
        return;
    }

    if (size < 0x10 || size > MAX_CHUNK_SIZE) {
        puts("invalid size");
        return;
    }

    chunks[index] = malloc((size_t)size);
    if (chunks[index] == NULL) {
        puts("malloc failed");
        exit(1);
    }

    chunk_sizes[index] = (size_t)size;
    memset(chunks[index], 'A' + (int)index, (size_t)size);

    puts("allocated");
}

static void free_chunk(void)
{
    unsigned long index = read_ulong("index: ");

    if (!valid_index(index) || chunks[index] == NULL) {
        puts("invalid index");
        return;
    }

    free(chunks[index]);

    /*
     * Intentional bug:
     * chunks[index] and chunk_sizes[index] are left unchanged.
     * The slot still contains a dangling pointer.
     */
    puts("freed");
}

static void show_chunk(void)
{
    unsigned long index = read_ulong("index: ");

    if (!valid_index(index) || chunks[index] == NULL) {
        puts("invalid index");
        return;
    }

    puts("DATA:");
    write(STDOUT_FILENO, chunks[index], SHOW_BYTES);
    putchar('\n');
}

__attribute__((noinline))
static void bof(void)
{
    char buf[64];

    puts("BOF INPUT:");
    read(STDIN_FILENO, buf, 256);
}

static void menu(void)
{
    puts("");
    puts("1. add");
    puts("2. free");
    puts("3. show");
    puts("4. bof");
    puts("5. exit");
}

int main(void)
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    while (1) {
        unsigned long choice;

        menu();
        choice = read_ulong("> ");

        switch (choice) {
        case 1:
            add_chunk();
            break;
        case 2:
            free_chunk();
            break;
        case 3:
            show_chunk();
            break;
        case 4:
            bof();
            break;
        case 5:
            puts("bye");
            return 0;
        default:
            puts("invalid choice");
            break;
        }
    }
}
