#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[0x20];
    void *ptr;
} Obj;

int main() {
    char *secret = malloc(0x30);
    strcpy(secret, "SECRET_HEAP_OBJECT");

    Obj *a = malloc(sizeof(Obj));
    Obj *alias = a;

    strcpy(a->name, "AAAA");
    a->ptr = NULL;

    printf("secret = %p\n", secret);
    printf("a      = %p\n", a);
    printf("alias  = %p\n", alias);

    free(a);
    a = NULL;

    puts("\n[+] freed a and set a = NULL");

    Obj *b = malloc(sizeof(Obj));
    strcpy(b->name, "BBBB");
    b->ptr = secret;

    printf("b      = %p\n", b);
    printf("b->ptr = %p\n", b->ptr);

    puts("\n[+] UAF read through alias");
    printf("alias->ptr = %p\n", alias->ptr);
    printf("leaked string = %s\n", (char *)alias->ptr);

    return 0;
}
