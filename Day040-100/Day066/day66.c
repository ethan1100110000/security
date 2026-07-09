#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[0x20];
    int is_admin;
} User;

void check(User *u) {
    if (u->is_admin) {
        puts("[+] admin access granted");
    } else {
        puts("[-] access denied");
    }
}

int main() {
    User *a = malloc(sizeof(User));
    User *alias = a;

    strcpy(a->name, "AAAA");
    a->is_admin = 0;

    printf("a     = %p\n", a);
    printf("alias = %p\n", alias);

    free(a);
    a = NULL;

    puts("\n[+] freed a and set a = NULL");

    User *b = malloc(sizeof(User));
    strcpy(b->name, "normal_user");
    b->is_admin = 0;

    printf("b     = %p\n", b);
    printf("b->is_admin = %d\n", b->is_admin);

    puts("\n[+] before UAF overwrite");
    check(b);

    puts("\n[+] UAF overwrite through alias");
    alias->is_admin = 1;

    printf("b->is_admin = %d\n", b->is_admin);

    puts("\n[+] after UAF overwrite");
    check(b);

    return 0;
}
