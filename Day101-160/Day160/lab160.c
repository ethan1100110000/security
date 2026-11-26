#define _GNU_SOURCE
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv);

/* Day160: educational, owned-local-process protection experiment.
 * The GOT probe writes back the SAME VALUE in a forked child.
 * The stack probe executes a single x86-64 RET byte, also in a child.
 * No shell or nonlocal target is involved.
 */
static void marker(void) __attribute__((noinline, used));
static void marker(void) { puts("LOCAL_MARKER"); }

static uintptr_t parse_hex(const char *s) {
    errno = 0;
    char *end = NULL;
    unsigned long long value = strtoull(s, &end, 0);
    if (errno || !end || end == s || *end) {
        fprintf(stderr, "invalid hexadecimal address/offset: %s\n", s);
        exit(2);
    }
    return (uintptr_t)value;
}

static void report_child(const char *label, pid_t pid) {
    int status = 0;
    if (waitpid(pid, &status, 0) != pid) {
        perror("waitpid");
        exit(2);
    }
    if (WIFSIGNALED(status))
        printf("%s: signal %d (%s)\n", label, WTERMSIG(status),
               strsignal(WTERMSIG(status)));
    else if (WIFEXITED(status))
        printf("%s: exit %d\n", label, WEXITSTATUS(status));
    else
        printf("%s: unexpected child status 0x%x\n", label, status);
}

static void stack_probe(void) {
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); exit(2); }
    if (pid == 0) {
#if !defined(__x86_64__)
        _exit(4);
#else
        volatile unsigned char stack_stub[] = {0xC3}; /* x86-64 RET */
        void (*fn)(void) = (void (*)(void))(uintptr_t)&stack_stub[0];
        fn(); /* architecture-specific NX page-permission probe */
        _exit(0);
#endif
    }
    report_child("stack_probe", pid);
}

static void got_probe(uintptr_t main_static_value, uintptr_t puts_got_rva) {
    uintptr_t base = (uintptr_t)&main - main_static_value;
    uintptr_t slot_address = base + puts_got_rva;
    printf("calculated_load_base=0x%" PRIxPTR "\n", base);
    printf("puts_got_slot=0x%" PRIxPTR "\n", slot_address);

    /* Only use puts R_X86_64_JUMP_SLOT from the SAME binary. */
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); exit(2); }
    if (pid == 0) {
        volatile uintptr_t *slot = (volatile uintptr_t *)slot_address;
        uintptr_t before = *slot;
        *slot = before; /* same-value write: page permission only */
        _exit(0);
    }
    report_child("got_same_value_write", pid);
}

int main(int argc, char **argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    if (argc < 2) {
        fprintf(stderr,
                "usage: %s addr | stack | got <main_sym_value> <puts_JUMP_SLOT_offset>\n",
                argv[0]);
        return 2;
    }
    if (strcmp(argv[1], "addr") == 0 && argc == 2) {
        printf("main=0x%" PRIxPTR " marker=0x%" PRIxPTR "\n",
               (uintptr_t)&main, (uintptr_t)&marker);
        return 0;
    }
    if (strcmp(argv[1], "stack") == 0 && argc == 2) {
        stack_probe();
        return 0;
    }
    if (strcmp(argv[1], "got") == 0 && argc == 4) {
        got_probe(parse_hex(argv[2]), parse_hex(argv[3]));
        return 0;
    }
    fprintf(stderr, "unknown/invalid args\n");
    return 2;
}
