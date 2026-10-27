#define _GNU_SOURCE
#include <link.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

unsigned char marker[8] = {0x41,0x00,0x42,0x43,0x10,0x20,0x30,0x40};
unsigned char zero_area[16];
static int checked, failed;
static int inspect_main(struct dl_phdr_info *info, size_t size, void *unused) {
    (void)size; (void)unused;
    if (info->dlpi_name[0]) return 0;
    uintptr_t addr=(uintptr_t)marker, bias=info->dlpi_addr;
    for (ElfW(Half) i=0;i<info->dlpi_phnum;i++) {
        const ElfW(Phdr) *p=&info->dlpi_phdr[i];
        if (p->p_type!=PT_LOAD) continue;
        uintptr_t start=bias+p->p_vaddr;
        if (addr<start || addr-start>=p->p_filesz) continue;
        uintptr_t delta=addr-start;
        if (p->p_filesz-delta<sizeof(marker)) continue;
        unsigned char file_bytes[sizeof(marker)];
        off_t off=(off_t)(p->p_offset+delta);
        int fd=open("/proc/self/exe",O_RDONLY);
        if(fd<0) {perror("open");failed=1;return 1;}
        ssize_t n=pread(fd,file_bytes,sizeof(file_bytes),off);
        close(fd);
        if(n!=(ssize_t)sizeof(file_bytes)) {failed=1;return 1;}
        printf("load_bias=0x%" PRIxPTR "\nmarker runtime address=0x%" PRIxPTR "\nmarker ELF address=0x%" PRIxPTR "\n",bias,addr,addr-bias);
        printf("LOAD offset=0x%jx vaddr=0x%jx\nmarker file offset=0x%jx\n",(uintmax_t)p->p_offset,(uintmax_t)p->p_vaddr,(uintmax_t)off);
        printf("file bytes:   ");for(size_t k=0;k<sizeof(marker);k++)printf("%02x ",file_bytes[k]);puts("");
        printf("memory bytes: ");for(size_t k=0;k<sizeof(marker);k++)printf("%02x ",marker[k]);puts("");
        int initial=memcmp(file_bytes,marker,sizeof(marker))==0;
        marker[0]=0x5a;
        int changed=marker[0]==0x5a && file_bytes[0]==0x41;
        printf("initial comparison=%s\nafter write: memory[0]=%02x saved file byte[0]=%02x\n",initial?"PASS":"FAIL",marker[0],file_bytes[0]);
        unsigned char reread=0;
        fd=open("/proc/self/exe",O_RDONLY);
        if(fd<0) {failed=1;return 1;}
        n=pread(fd,&reread,1,off);close(fd);
        int unchanged=n==1 && reread==0x41;
        printf("file reread[0]=%02x unchanged=%s\n",reread,unchanged?"PASS":"FAIL");
        checked=1; failed=!(initial && changed && unchanged);
        return 1;
    }
    return 1;
}
int main(void) {
    dl_iterate_phdr(inspect_main,0);
    int zero=1;for(size_t i=0;i<sizeof(zero_area);i++)if(zero_area[i])zero=0;
    printf("zero_area initial 16 bytes all zero=%s\n",zero?"PASS":"FAIL");
    return !checked || failed || !zero;
}
