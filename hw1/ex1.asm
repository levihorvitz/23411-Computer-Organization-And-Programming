.global _start

.section .text
_start:
    movq num , %rax
    movl $0x0 , countBits
loop1:
    cmp $0x0 , %rax
    je end
    shr %rax
    adc $0x0 , countBits
    jmp loop1
end: