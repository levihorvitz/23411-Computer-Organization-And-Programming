.global hook

.section .data
msg: .ascii "This code was hacked by Noa Killa's gang\n"
endmsg:

.section .text
hook:
  lea _start,%rax
  movb  $0xc3, 0x1e(%rax)
  lea mergi , %rdx
  add $0x26 , %rax
  push %rax
  push %rdx
  jmp _start
mergi:
  mov $0x1 , %rax
  mov $0x1 , %rdi
  lea msg , %rsi
  lea endmsg-msg , %rdx
  syscall
  ret
  movq $60, %rax
  movq $0, %rdi
  syscall
  
