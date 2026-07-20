#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char name[32];
    void (*callback)(const char *);
} Handler;

int main(void)
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    Handler *old = malloc(sizeof(Handler));

    memset(old, 0, sizeof(Handler));
    strcpy(old->name, "old object");
    old->callback = puts;

    free(old);

    Handler *fresh = malloc(sizeof(Handler));

    puts("name:");
    read(STDIN_FILENO, fresh->name, 8);

    puts("raw record:");
    write(STDOUT_FILENO, fresh, sizeof(Handler));

    return 0;
}
