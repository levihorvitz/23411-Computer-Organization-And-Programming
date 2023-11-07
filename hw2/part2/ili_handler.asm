.globl my_ili_handler
.extern what_to_do, old_ili_handler

.text
.align 4, 0x90
my_ili_handler:
    xor %rax, %rax
    push %r10
    push %rdi
    movq 16(%rsp),%r10
    movw (%r10), %di
    cmpb $0x0f , %dil
    jne next_hw2
    shr $8 , %di
    inc %r10

next_hw2:
    inc %r10
    push %rsi
    push %rdx
    push %rcx
    push %r8
    push %r9
    push %r10
    push %r11
    call what_to_do
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rcx
    pop %rdx
    pop %rsi
    cmp $0 , %rax
    je j_to_old_handler_hw2
    movq %r10 , 16(%rsp)
    pop %rdi
    mov %rax , %rdi
    pop %r10
    iretq
    
j_to_old_handler_hw2: 
    pop %rdi
    pop %r10
    jmp *old_ili_handler