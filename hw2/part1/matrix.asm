.global get_elemnt_from_matrix, multiplyMatrices
.extern set_elemnt_in_matrix

.section .text
get_elemnt_from_matrix:
        xor %rax , %rax
	mov %edx,%eax
        mul %esi
        add %ecx,%eax
        adc $0x0,%edx
        sal $32,%rdx
        add %rdx,%rax
        movl (%rdi,%rax,4) , %eax
	ret

multiplyMatrices:
	pushq %rbp
        movq %rsp , %rbp
        
        push %r12
        push %r13
        push %r14
        push %rbx

        push %r9 #rsp+40
        push %r8 #rsp+32
        push %rcx #rsp+24
        push %rdx #rsp+16
        push %rsi #rsp+8
        push %rdi #rsp       
        
         
        xor %r12d,%r12d
loop_i: 
        xor %r14d,%r14d
loop_j:
        xor %r13d,%r13d
        xor %r10,%r10
loop_k:
        
        #get A[i][k]
        movq (%rsp),%rdi
        movl 32(%rsp),%esi
        movl %r12d,%edx
        movl %r13d,%ecx
        call get_elemnt_from_matrix
        
        movl %eax,%ebx
        
        #get B[k][j]
        movq 8(%rsp),%rdi
        movl 40(%rsp),%esi
        movl %r13d,%edx
        movl %r14d,%ecx
        call get_elemnt_from_matrix

        #mul and add to sum
        movl %r10d,%edi
        movl %eax,%esi
        movl %ebx,%edx
        movl 16(%rbp),%ecx
        call mul_and_add_modp
        movl %eax , %r10d      
        
        inc %r13d
        cmp %r13d,32(%rsp)
        jne loop_k
        #end of loop_k
        
        # Matrix[%r12][%r14] = %r10
        movq 16(%rsp),%rdi
        movl 40(%rsp),%esi
        movl %r12d,%edx
        movl %r14d,%ecx
        movl %r10d,%r8d
        call set_elemnt_in_matrix


        inc %r14d
        cmp %r14d,40(%rsp)
        jne loop_j
        #end of loop_j
        
        inc %r12d
        cmp %r12d,24(%rsp)
        jne loop_i
        #end of loop_i
        
        add $48,%rsp
            
        pop %rbx
        pop %r14
        pop %r13
        pop %r12
        
        popq %rbp
	ret


mul_and_add_modp: #int f(int sum , int a, int b, int p) --> return sum + a*b (mod p)
        movl %edx,%eax
        mul %esi
        addl %edi,%eax
        adc $0x0,%edx
        div %ecx
        movl %edx , %eax
        ret
