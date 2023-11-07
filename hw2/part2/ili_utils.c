#include <asm/desc.h>

void my_store_idt(struct desc_ptr *idtr) {
// <STUDENT FILL>
	asm ("sidt %0\n" :"=m"(*idtr));
// </STUDENT FILL>
}

void my_load_idt(struct desc_ptr *idtr) {
// <STUDENT FILL>
	asm ("lidt %0\n" : :"m"(*idtr));
// <STUDENT FILL>
}

void my_set_gate_offset(gate_desc *gate, unsigned long addr) {
// <STUDENT FILL>
	asm volatile("movw %%ax,(%0);"
				"shr $16,%%rax;"
				"movw %%ax , 6(%0);"
				"shr $16,%%rax;"
				"movl %%eax, 8(%0);"
				: 
				:"g" (gate) , "a"(addr)
				);
// </STUDENT FILL>
}

unsigned long my_get_gate_offset(gate_desc *gate) {
// <STUDENT FILL>
	unsigned long offset=0;
	asm volatile( "movq 4(%1) , %%rax;"
				"movw (%1), %%ax;"
				:"=a"(offset)
				:"g"(gate)
				);
	return offset;
// </STUDENT FILL>
}
