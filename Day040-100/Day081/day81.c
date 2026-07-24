#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

static void fake_hook(void *ptr, const void *caller)
{
    (void)ptr;
    (void)caller;
    puts("[+] HOOK CALLED");
}

static void write8(void *where, unsigned long value)
{
    *(unsigned long *)where = value;
}

static char command[] = "/bin/sh";
static int (*action)(const char *) = puts;


int main(void)
{
    void *p = malloc(0x20);

    void **hook = (void **)dlvsym(
        RTLD_DEFAULT,
        "__free_hook",
        "GLIBC_2.2.5"
    );

    printf("__free_hook address = %p\n", (void *)hook);

    if (hook == NULL) {
        printf("dlvsym failed: %s\n", dlerror());
        return 1;
    }

    *hook = (void *)fake_hook;
    printf("__free_hook value   = %p\n", *hook);

    free(p);
    puts("[+] after free");

    printf("action address = %p\n", (void *)&action);
    printf("action before  = %p\n", (void *)action);

    write8(&action, (unsigned long)system);

    printf("action after   = %p\n", (void *)action);
    action(command);
    return 0;
}
