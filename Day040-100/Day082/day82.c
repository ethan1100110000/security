#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    uint64_t fake_stack[10];
    void (*callback)(void *);
} Object;

static Object *obj;

__attribute__((naked))
static void gadget_pop_rdi(void)
{
    __asm__("pop %rdi; ret");
}

__attribute__((naked))
static void gadget_ret(void)
{
    __asm__("ret");
}

__attribute__((naked))
static void trigger_object(Object *p)
{
    __asm__(
        "mov %rdi, %rbp;"
        "call *0x50(%rdi);"
        "ret;"
    );
}

static void safe_callback(void *p)
{
    puts("safe callback");
}

int main(void)
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    
    void *region = malloc(0x5000);
    obj = (Object *)((char *)region + 0x4000);
    
    obj->callback = safe_callback;

    printf("heap object: %p\n", (void *)obj);
    printf("system: %p\n", (void *)system);
    printf("exit: %p\n", (void *)exit);

    puts("input object data:");
    read(0, obj, sizeof(Object));

    trigger_object(obj);
    return 0;
}
