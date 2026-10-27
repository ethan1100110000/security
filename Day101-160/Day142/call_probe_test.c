#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
extern void call_probe(uint64_t out[12]);
int main(void) {
    uint64_t o[12]={0};
    call_probe(o);
    printf("baseline rsp=0x%" PRIx64 " rbx=55\n",o[0]);
    printf("after push rsp=0x%" PRIx64 "\n",o[1]);
    printf("helper entry rsp=0x%" PRIx64 " [rsp]=0x%" PRIx64 " [rsp+8]=%" PRIu64 "\n",o[2],o[3],o[4]);
    printf("after sub16 rsp=0x%" PRIx64 " [rsp+16]=0x%" PRIx64 "\n",o[5],o[6]);
    printf("after helper ret rsp=0x%" PRIx64 " rbx=%" PRIu64 "\n",o[7],o[8]);
    printf("after pop rsp=0x%" PRIx64 " rbx=%" PRIu64 "\n",o[9],o[10]);
    int stack=o[1]==o[0]-8 && o[2]==o[0]-16 && o[5]==o[0]-32 && o[7]==o[1] && o[9]==o[0];
    int saved=o[4]==55 && o[8]==88 && o[10]==55;
    int retaddr=o[3]==o[11] && o[6]==o[11];
    int alignment=o[1]%16==0 && o[2]%16==8;
    printf("stack=%s saved=%s return-address=%s alignment=%s\n",stack?"PASS":"FAIL",saved?"PASS":"FAIL",retaddr?"PASS":"FAIL",alignment?"PASS":"FAIL");
    return !(stack && saved && retaddr && alignment);
}
