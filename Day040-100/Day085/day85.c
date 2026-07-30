#define _GNU_SOURCE
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SMALL_REQUEST 0x60
#define LARGE_REQUEST 0x500
#define SMALL_SLOTS 8
#define LARGE_SLOTS 4

typedef struct {
    unsigned char data[0x48];
    void *self;
    void (*callback)(void);
    unsigned char reserved[0x08];
} SmallObject;

_Static_assert(sizeof(SmallObject) == SMALL_REQUEST, "SmallObject size must be 0x60");

static SmallObject *small_slots[SMALL_SLOTS];
static unsigned char *large_slots[LARGE_SLOTS];

#define CONTROL_OFFSET 0x4000
#define CONTROL_REGION_SIZE 0x5000

__attribute__((aligned(0x1000)))
unsigned char control_region[CONTROL_REGION_SIZE];

__attribute__((noinline))
void default_callback(void) {
    puts("default callback");
}

__attribute__((naked, noinline, used, no_stack_protector))
void gadget_pop_rdi_ret(void) {
    __asm__("pop %rdi; ret");
}

__attribute__((naked, noinline, used, no_stack_protector))
void gadget_ret(void) {
    __asm__("ret");
}

__attribute__((naked, noinline, used, no_stack_protector))
void gadget_leave_ret(void) {
    __asm__("leave; ret");
}

__attribute__((naked, noinline, no_stack_protector))
static void trigger_object(void *obj __attribute__((unused))) {
    __asm__(
        "push %rbp\n"
        "mov %rdi, %rbp\n"
        "call *0x50(%rdi)\n"
        "pop %rbp\n"
        "ret\n"
    );
}

static void die(const char *msg) {
    perror(msg);
    _exit(1);
}

static unsigned long read_number(const char *prompt) {
    char buf[0x40];
    char *end = NULL;

    fputs(prompt, stdout);
    if (fgets(buf, sizeof(buf), stdin) == NULL) {
        _exit(0);
    }

    errno = 0;
    unsigned long value = strtoul(buf, &end, 0);
    if (errno != 0 || end == buf) {
        puts("invalid number");
        return (unsigned long)-1;
    }
    return value;
}

static void read_once(void *dst, size_t size) {
    ssize_t n = read(STDIN_FILENO, dst, size);
    if (n < 0) {
        die("read");
    }
}

static int valid_small(unsigned long idx) {
    if (idx >= SMALL_SLOTS || small_slots[idx] == NULL) {
        puts("invalid small slot");
        return 0;
    }
    return 1;
}

static int valid_large(unsigned long idx) {
    if (idx >= LARGE_SLOTS || large_slots[idx] == NULL) {
        puts("invalid large slot");
        return 0;
    }
    return 1;
}

static void add_small(void) {
    unsigned long idx = read_number("small index: ");
    if (idx >= SMALL_SLOTS) {
        puts("invalid small slot");
        return;
    }

    SmallObject *obj = malloc(SMALL_REQUEST);
    if (obj == NULL) {
        die("malloc");
    }

    small_slots[idx] = obj;
    obj->self = obj;
    obj->callback = default_callback;

    fputs("small data: ", stdout);
    read_once(obj->data, sizeof(obj->data));
    puts("small added");
}

static void delete_small(void) {
    unsigned long idx = read_number("small index: ");
    if (!valid_small(idx)) {
        return;
    }

    free(small_slots[idx]);
    puts("small deleted");
}

static void show_small(void) {
    unsigned long idx = read_number("small index: ");
    if (!valid_small(idx)) {
        return;
    }

    fputs("small raw: ", stdout);
    if (write(STDOUT_FILENO, small_slots[idx], SMALL_REQUEST) < 0) {
        die("write");
    }
    putchar('\n');
}

static void edit_small(void) {
    unsigned long idx = read_number("small index: ");
    if (!valid_small(idx)) {
        return;
    }

    fputs("small raw data: ", stdout);
    read_once(small_slots[idx], SMALL_REQUEST);
    puts("small edited");
}

static void add_large(void) {
    unsigned long idx = read_number("large index: ");
    if (idx >= LARGE_SLOTS) {
        puts("invalid large slot");
        return;
    }

    unsigned char *buf = malloc(LARGE_REQUEST);
    if (buf == NULL) {
        die("malloc");
    }

    large_slots[idx] = buf;
    fputs("large data: ", stdout);
    read_once(buf, LARGE_REQUEST);
    puts("large added");
}

static void delete_large(void) {
    unsigned long idx = read_number("large index: ");
    if (!valid_large(idx)) {
        return;
    }

    free(large_slots[idx]);
    puts("large deleted");
}

static void show_large(void) {
    unsigned long idx = read_number("large index: ");
    if (!valid_large(idx)) {
        return;
    }

    fputs("large raw: ", stdout);
    if (write(STDOUT_FILENO, large_slots[idx], 0x20) < 0) {
        die("write");
    }
    putchar('\n');
}

static void trigger_control(void) {
    trigger_object(control_region + CONTROL_OFFSET);
}

static void menu(void) {
    puts("\n1. add small");
    puts("2. delete small");
    puts("3. show small");
    puts("4. edit small");
    puts("5. add large");
    puts("6. delete large");
    puts("7. show large");
    puts("8. trigger control object");
    puts("9. exit");
}

int main(void) {
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    memset(control_region, 0, sizeof(control_region));

    while (1) {
        menu();
        unsigned long choice = read_number("> ");

        switch (choice) {
        case 1:
            add_small();
            break;
        case 2:
            delete_small();
            break;
        case 3:
            show_small();
            break;
        case 4:
            edit_small();
            break;
        case 5:
            add_large();
            break;
        case 6:
            delete_large();
            break;
        case 7:
            show_large();
            break;
        case 8:
            trigger_control();
            break;
        case 9:
            return 0;
        default:
            puts("invalid choice");
            break;
        }
    }
}
