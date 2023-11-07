.global _start

.section .text
_start:
    xor %ebx, %ebx
    movl $1 , len
    movl $0x0 , begin
    movl $-4 , %edx
    movl n, %ecx #array size
    dec %ecx
    je end
loopa:
    inc %ebx
    movl arr(,%ecx,4) , %eax
    cmp arr(%edx,%ecx,4), %eax
p3:
    jl p1
    cmp len , %ebx
    jl p2
    movl %ebx , len
    movl %ecx , begin
p2:
    xor %ebx , %ebx
    cmp $0x0 , %ecx
    je end
p1:
    loop loopa
    inc %ebx
    jmp p3
end:

