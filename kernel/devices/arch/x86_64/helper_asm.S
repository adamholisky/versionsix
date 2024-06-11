.section .text

.global get_cr0
get_cr0:
	mov %cr0, %rax
	ret;

.global get_cr2
get_cr2:
	mov %cr2, %rax
	ret;

.global get_cr3
get_cr3:
	mov %cr3, %rax
	ret;

.global get_cr4
get_cr4:
	mov %cr4, %rax
	ret;

.global asm_refresh_cr3
.type asm_refresh_cr3, @function
asm_refresh_cr3:
	xor %rcx, %rcx
	mov %cr3, %rcx
	mov %rcx, %cr3
	ret