.global _start

.section .text
_start:
    cmpl $0x0, num
    jle end    
    xor %ecx, %ecx
    movl $source, %ebx
    movl $destination, %edx
loopy:
    movb (%ebx, %ecx, 0x1), %al
    movb %al, (%edx, %ecx, 0x1)
    inc %ecx
    cmp %ecx, num
    jne loopy
end: