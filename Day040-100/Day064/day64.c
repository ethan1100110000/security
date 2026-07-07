#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[0x30];
} User;

int main() {
    User *a = malloc(sizeof(User));
    User *alias = a;

    strcpy(a->name, "AAAA");
    printf("a     = %p\n", a);
    printf("alias = %p\n", alias);
    printf("a->name = %s\n", a->name);

    free(a);
    a = NULL;

    puts("\n[+] freed a and set a = NULL");
    printf("a     = %p\n", a);
    printf("alias = %p\n", alias);

    User *b = malloc(sizeof(User));
    strcpy(b->name, "BBBB");

    puts("\n[+] allocated b");
    printf("b     = %p\n", b);
    printf("b->name = %s\n", b->name);

    puts("\n[+] UAF write through alias");
    strcpy(alias->name, "HACKED_BY_ALIAS");

    printf("b->name = %s\n", b->name);

    return 0;
}
