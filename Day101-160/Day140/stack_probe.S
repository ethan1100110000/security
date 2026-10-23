.text
.globl stack_probe
.type stack_probe, @function
stack_probe:
    push %rbx
    mov $100, %ebx
    mov %rsp, 0(%rdi)
    mov %rbx, 8(%rdi)

    push %rbx
    mov $200, %ebx
    mov %rsp, 16(%rdi)
    mov %rbx, 24(%rdi)
    mov (%rsp), %rax
    mov %rax, 32(%rdi)

    pop %rbx
    mov %rsp, 40(%rdi)
    mov %rbx, 48(%rdi)

    pop %rbx
    ret
.size stack_probe, .-stack_probe
.section .note.GNU-stack,"",@progbits
