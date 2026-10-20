.text
.globl write_ecx
.type write_ecx, @function
write_ecx:
    movabs $0x1122334455667788, %rcx
    mov $1, %ecx
    mov %rcx, %rax
    ret
.size write_ecx, .-write_ecx

.globl write_cx
.type write_cx, @function
write_cx:
    movabs $0x1122334455667788, %rcx
    mov $1, %cx
    mov %rcx, %rax
    ret
.size write_cx, .-write_cx
.section .note.GNU-stack,"",@progbits
