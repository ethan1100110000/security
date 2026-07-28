#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    char data[0x20];
    void (*callback)(char *);
} Note;

static Note *slots[4];

static void read_n(void *buf, size_t size)
{
    size_t total = 0;

    while (total < size) {
        ssize_t n = read(STDIN_FILENO, (char *)buf + total, size - total);
        if (n <= 0)
            exit(1);
        total += (size_t)n;
    }
}

static int read_int(void)
{
    char buf[0x20] = {0};
    size_t i = 0;

    while (i < sizeof(buf) - 1) {
        char c;
        ssize_t n = read(STDIN_FILENO, &c, 1);
        if (n <= 0)
            exit(1);
        if (c == '\n')
            break;
        buf[i++] = c;
    }

    return atoi(buf);
}

static int read_index(void)
{
    puts("index:");
    int idx = read_int();

    if (idx < 0 || idx >= 4) {
        puts("invalid index");
        return -1;
    }

    return idx;
}

static void add_note(void)
{
    int idx = read_index();
    if (idx < 0)
        return;

    if (slots[idx] != NULL) {
        puts("occupied");
        return;
    }

    slots[idx] = malloc(sizeof(Note));
    if (slots[idx] == NULL)
        exit(1);

    puts("data:");
    read_n(slots[idx]->data, sizeof(slots[idx]->data));
    slots[idx]->callback = (void (*)(char *))puts;

    puts("added");
}

static void delete_note(void)
{
    int idx = read_index();
    if (idx < 0)
        return;

    if (slots[idx] == NULL) {
        puts("empty");
        return;
    }

    free(slots[idx]);

    /* Intentionally vulnerable: dangling pointer remains in slots[idx]. */
    puts("deleted");
}

static void show_note(void)
{
    int idx = read_index();
    if (idx < 0)
        return;

    if (slots[idx] == NULL) {
        puts("empty");
        return;
    }

    puts("raw:");
    write(STDOUT_FILENO, slots[idx], sizeof(Note));
    write(STDOUT_FILENO, "\n", 1);
}

static void edit_note(void)
{
    int idx = read_index();
    if (idx < 0)
        return;

    if (slots[idx] == NULL) {
        puts("empty");
        return;
    }

    puts("new data:");
    read_n(slots[idx], sizeof(Note));
    puts("edited");
}

static void trigger_note(void)
{
    int idx = read_index();
    if (idx < 0)
        return;

    if (slots[idx] == NULL) {
        puts("empty");
        return;
    }

    slots[idx]->callback(slots[idx]->data);
}

static void menu(void)
{
    puts("1. add");
    puts("2. delete");
    puts("3. show");
    puts("4. edit");
    puts("5. trigger");
    puts("6. exit");
    puts("> ");
}

int main(void)
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    while (1) {
        menu();

        switch (read_int()) {
        case 1:
            add_note();
            break;
        case 2:
            delete_note();
            break;
        case 3:
            show_note();
            break;
        case 4:
            edit_note();
            break;
        case 5:
            trigger_note();
            break;
        case 6:
            return 0;
        default:
            puts("invalid choice");
        }
    }
}
