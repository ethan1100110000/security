#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

extern void stack_probe(uint64_t out[7]);

int main(void)
{
    uint64_t out[7] = {0};
    stack_probe(out);
    printf("before experiment push: rsp=0x%016" PRIX64 " rbx=%" PRIu64 "\n", out[0], out[1]);
    printf("after push and rbx=200: rsp=0x%016" PRIX64 " rbx=%" PRIu64 " [rsp]=%" PRIu64 "\n", out[2], out[3], out[4]);
    printf("after experiment pop: rsp=0x%016" PRIX64 " rbx=%" PRIu64 "\n", out[5], out[6]);
    int push_ok = out[0] >= 8 && out[2] == out[0] - 8;
    int pop_ok = out[5] == out[0];
    int values_ok = out[1] == 100 && out[3] == 200 && out[4] == 100 && out[6] == 100;
    printf("push rsp -8: %s; pop restores rsp: %s; values: %s\n",
           push_ok ? "PASS" : "FAIL", pop_ok ? "PASS" : "FAIL", values_ok ? "PASS" : "FAIL");
    return push_ok && pop_ok && values_ok ? 0 : 1;
}
