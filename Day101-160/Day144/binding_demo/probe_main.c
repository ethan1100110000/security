#define _GNU_SOURCE
#include <link.h>
#include <dlfcn.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
extern int lesson_helper(void);
static uintptr_t slot_address;
static uintptr_t load_bias;
static int find_slot(struct dl_phdr_info *info, size_t size, void *unused)
{
    (void)size; (void)unused;
    if (info->dlpi_name[0]) return 0;
    load_bias = info->dlpi_addr;
    const ElfW(Dyn) *dynamic = NULL;
    for (ElfW(Half) i=0; i<info->dlpi_phnum; ++i)
        if (info->dlpi_phdr[i].p_type == PT_DYNAMIC)
            dynamic = (const ElfW(Dyn) *)(load_bias + info->dlpi_phdr[i].p_vaddr);
    if (!dynamic) return 1;
    const ElfW(Rela) *rela = NULL;
    const ElfW(Sym) *symbols = NULL;
    const char *strings = NULL;
    size_t bytes = 0;
    long kind = 0;
    for (const ElfW(Dyn) *d=dynamic; d->d_tag != DT_NULL; ++d) {
        if (d->d_tag == DT_JMPREL) rela=(const ElfW(Rela) *)d->d_un.d_ptr;
        if (d->d_tag == DT_PLTRELSZ) bytes=d->d_un.d_val;
        if (d->d_tag == DT_SYMTAB) symbols=(const ElfW(Sym) *)d->d_un.d_ptr;
        if (d->d_tag == DT_STRTAB) strings=(const char *)d->d_un.d_ptr;
        if (d->d_tag == DT_PLTREL) kind=d->d_un.d_val;
    }
    if (!rela || !symbols || !strings || kind != DT_RELA) return 1;
    for (size_t i=0; i<bytes/sizeof(*rela); ++i) {
        size_t sym=ELF64_R_SYM(rela[i].r_info);
        if (!strcmp(strings+symbols[sym].st_name,"lesson_helper")) {
            slot_address=load_bias+rela[i].r_offset;
            break;
        }
    }
    return 1;
}
static uintptr_t snapshot(const char *when)
{
    uintptr_t target;
    memcpy(&target,(const void *)slot_address,sizeof(target));
    Dl_info owner={0};
    dladdr((const void *)target,&owner);
    printf("%s target=0x%" PRIxPTR " owner=%s\n",when,target,owner.dli_fname?owner.dli_fname:"unknown");
    return target;
}
static void print_slot_mapping(void)
{
    FILE *maps=fopen("/proc/self/maps","r");
    if (!maps) { puts("slot mapping unavailable"); return; }
    char line[512];
    while (fgets(line,sizeof(line),maps)) {
        unsigned long begin,end;
        if (sscanf(line,"%lx-%lx",&begin,&end)==2 && slot_address>=begin && slot_address<end) {
            printf("slot mapping: %s",line); break;
        }
    }
    fclose(maps);
}
int main(void)
{
    dl_iterate_phdr(find_slot,NULL);
    if (!slot_address) { fputs("lesson_helper JUMP_SLOT not found\n",stderr); return 1; }
    printf("load_bias=0x%" PRIxPTR " slot ELF VA=0x%" PRIxPTR " runtime slot=0x%" PRIxPTR "\n",load_bias,slot_address-load_bias,slot_address);
    print_slot_mapping();
    uintptr_t before=snapshot("before");
    int first=lesson_helper();
    uintptr_t after_first=snapshot("after first");
    int second=lesson_helper();
    uintptr_t after_second=snapshot("after second");
    printf("results=%d,%d changed_on_first=%s stable_on_second=%s\n",first,second,before!=after_first?"YES":"NO",after_first==after_second?"YES":"NO");
    return !(first==7 && second==7 && after_first==after_second);
}
