#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

extern uint64_t write_ecx(void);
extern uint64_t write_cx(void);

int main(void)
{
    uint64_t ecx_result = write_ecx();
    uint64_t cx_result = write_cx();
    int ecx_ok = ecx_result == UINT64_C(0x0000000000000001);
    int cx_ok = cx_result == UINT64_C(0x1122334455660001);
    printf("initial rcx = 0x1122334455667788 (reset before each case)\n");
    printf("mov ecx, 1 -> rcx = 0x%016" PRIX64 " expected=0x0000000000000001 %s\n",
           ecx_result, ecx_ok ? "PASS" : "FAIL");
    printf("mov cx, 1  -> rcx = 0x%016" PRIX64 " expected=0x1122334455660001 %s\n",
           cx_result, cx_ok ? "PASS" : "FAIL");
    return ecx_ok && cx_ok ? 0 : 1;
}
