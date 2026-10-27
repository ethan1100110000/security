.text
.globl call_probe
.type call_probe,@function
call_probe:
    push %rbx
    sub $8,%rsp
    mov $55,%ebx
    mov %rsp,0(%rdi)
    push %rbx
    mov $88,%ebx
    mov %rsp,8(%rdi)
    lea .Lresume(%rip),%rax
    mov %rax,88(%rdi)
    call .Lhelper
.Lresume:
    mov %rsp,56(%rdi)
    mov %rbx,64(%rdi)
    pop %rbx
    mov %rsp,72(%rdi)
    mov %rbx,80(%rdi)
    add $8,%rsp
    pop %rbx
    ret
.Lhelper:
    mov %rsp,16(%rdi)
    mov (%rsp),%rax
    mov %rax,24(%rdi)
    mov 8(%rsp),%rax
    mov %rax,32(%rdi)
    sub $16,%rsp
    mov %rsp,40(%rdi)
    mov 16(%rsp),%rax
    mov %rax,48(%rdi)
    add $16,%rsp
    ret
.size call_probe,.-call_probe
.section .note.GNU-stack,"",@progbits
