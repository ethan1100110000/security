#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static void dump_chunk(const char *name, void *p) {
    uintptr_t *hdr = (uintptr_t *)p - 2;
    uintptr_t prev_size = hdr[0];
    uintptr_t size_flags = hdr[1];
    uintptr_t real_size = size_flags & ~(uintptr_t)0xf;
    uintptr_t flags = size_flags & 0xf;

    printf("\n[%s]\n", name);
    printf("user pointer          : %p\n", p);
    printf("chunk metadata start  : %p\n", (void *)hdr);
    printf("prev_size             : 0x%lx\n", prev_size);
    printf("size | flags          : 0x%lx\n", size_flags);
    printf("real chunk size       : 0x%lx\n", real_size);
    printf("flags                 : 0x%lx\n", flags);
    printf("PREV_INUSE            : %lu\n", size_flags & 1UL);
    printf("next chunk metadata   : %p\n", (void *)((char *)hdr + real_size));
}

int main(void) {
    setbuf(stdout, NULL);

    puts("=== Day62 Heap Metadata Lab ===");
    puts("Goal: observe glibc chunk metadata: prev_size, size|flags, real size, PREV_INUSE");

    char *a = malloc(0x30);
    char *b = malloc(0x30);
    char *c = malloc(0x80);

    if (!a || !b || !c) {
        puts("malloc failed");
        return 1;
    }

    strcpy(a, "AAAA");
    strcpy(b, "BBBB");
    strcpy(c, "CCCC");

    printf("\nAllocated chunks:\n");
    printf("a = %p  malloc(0x30)\n", a);
    printf("b = %p  malloc(0x30)\n", b);
    printf("c = %p  malloc(0x80)\n", c);

    dump_chunk("a", a);
    dump_chunk("b", b);
    dump_chunk("c", c);

    puts("\nUseful gdb commands:");
    puts("  x/2gx a-0x10");
    puts("  x/2gx b-0x10");
    puts("  x/gx  a-0x8");
    puts("  x/gx  a+0x38      # next chunk size|flags when a chunk size is 0x40");

    puts("\nPress Enter to free(a) and malloc(0x30) again...");
    getchar();

    free(a);
    char *d = malloc(0x30);

    printf("\nAfter free(a); d = malloc(0x30);\n");
    printf("d = %p\n", d);
    puts("If tcache reused the last freed 0x30 chunk, d should equal old a.");

    dump_chunk("d", d);
    dump_chunk("b", b);

    puts("\nPress Enter to exit...");
    getchar();

    free(d);
    free(b);
    free(c);
    return 0;
}
