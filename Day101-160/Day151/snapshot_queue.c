#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    unsigned id;
    char title[32];
    char detail[80];
} Snapshot;

static Snapshot *active;
static Snapshot *last_preview;
static unsigned next_id = 1;

static void read_line(char *dst, size_t cap) {
    if (!fgets(dst, cap, stdin)) {
        puts("input closed");
        exit(0);
    }
    dst[strcspn(dst, "\n")] = '\0';
}

static void create_snapshot(void) {
    Snapshot *new_snapshot = calloc(1, sizeof(*new_snapshot));
    if (!new_snapshot) {
        perror("calloc");
        exit(1);
    }
    new_snapshot->id = next_id++;

    printf("title> ");
    fflush(stdout);
    read_line(new_snapshot->title, sizeof(new_snapshot->title));

    printf("detail> ");
    fflush(stdout);
    read_line(new_snapshot->detail, sizeof(new_snapshot->detail));

    free(active);
    active = new_snapshot;
    last_preview = new_snapshot;
    printf("created #%u\n", active->id);
}

static void close_snapshot(void) {
    if (!active) {
        puts("nothing to close");
        return;
    }
    free(active);
    active = NULL;
    puts("closed");
}

static void show_latest(void) {
    if (!last_preview) {
        puts("no recent snapshot");
        return;
    }
    printf("#%u: %s\n", last_preview->id, last_preview->title);
    printf("%s\n", last_preview->detail);
}

int main(void) {
    char choice[16];
    puts("Snapshot Queue / local study sample");
    for (;;) {
        puts("\n1) New  2) Close  3) Recent  4) Exit");
        printf("> ");
        fflush(stdout);
        read_line(choice, sizeof(choice));
        switch (choice[0]) {
            case '1': create_snapshot(); break;
            case '2': close_snapshot(); break;
            case '3': show_latest(); break;
            case '4': free(active); return 0;
            default: puts("unknown command"); break;
        }
    }
}
