.global _start

.section .text
_start:
    movq $head , %rcx 
    movq head , %r8
    movq src, %rax
    jmp p2
p1:
    lea 8(%r8) , %rcx
    movq (%rcx), %r8
p2: 
    cmpq (%r8), %rax
    jne p1
    lea 8(%r8), %rdx
    movq 8(%r8) , %r9
    movq dst, %rax
    jmp p4
p3:
    lea 8(%r9) , %rdx
    movq (%rdx), %r9
p4:
    cmpq $0x0 , %r9
    je end
    cmpq (%r9), %rax
    jne p3
    movq (%rcx),%rax
    movq (%rdx) ,%rbx
    movq %rbx , (%rcx)
    movq %rax , (%rdx)
    movq 8(%r8),%rax
    movq 8(%r9) ,%rbx
    movq %rbx , 8(%r8)
    movq %rax ,8(%r9)    
end:    
            
    
    