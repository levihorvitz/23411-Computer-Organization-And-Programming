.global _start

.section .text
_start:

    cmpl $0x0 , arr
    je end
    xor %rax, %rax
    xor %rcx, %rcx
    xor %rdx, %rdx
    
loopo:
    movl arr( ,%rcx,4) ,%ebx
    add %rbx , %rax
    adc $0x0 , %rdx
    inc %rcx
    cmpl $0x0 , arr( ,%rcx,4)
    jne loopo
    
    div %rcx
    movl %eax , avg

end: