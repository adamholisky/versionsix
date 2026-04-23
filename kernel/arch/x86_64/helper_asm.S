.section .data

dyn_linker_func_ret_addr: .quad 0x0

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

.global set_cr3
set_cr3:
	mov %rdi, %cr3
	ret

#define CR0_FPU_EM (1 << 2)
#define CR0_FPU_EXCP (1 << 5)
#define CR4_SSE_ENABLE (1 << 9)
#define CR4_SSE_EXCP (1 << 10)

.global sse_initalize
.type sse_initalize, @function
sse_initalize:
	xor %rax, %rax
	mov %cr0, %rax
	and $(~CR0_FPU_EM), %rax
	or $CR0_FPU_EXCP, %rax
	mov %rax, %cr0
	fninit

	xor %rax, %rax
	mov %cr4, %rax
	or $(CR4_SSE_ENABLE | CR4_SSE_EXCP), %rax
	mov %rax, %cr4

	push $0x1F80
	ldmxcsr (%rsp)
	fxsave fxsave_region
	pop %rax
	ret

.extern elf_loader_dynamic_linker
.global elf_dynamic_linker_preamble
.type elf_dynamic_linker_preamble, @function
elf_dynamic_linker_preamble:
	push %r15
	push %r14
	push %r13
	push %r12
	push %r11
	push %r10
	push %r9
	push %r8
		push %rbp
		push %rdi
		push %rsi
			push %rdx
			push %rcx
			push %rbx
			push %rax
				push %rsp
					movq 128(%rsp), %rdi
					movq 136(%rsp), %rsi
					call elf_loader_dynamic_linker
					movq %rax, dyn_linker_func_ret_addr
				pop %rsp
			pop %rax
			pop %rbx
			pop %rcx
			pop %rdx
		pop %rsi
		pop %rdi
		pop %rbp
	pop %r8
	pop %r9
	pop %r10
	pop %r11
	pop %r12
	pop %r13
	pop %r14
	pop %r15

	add $16, %rsp
	push dyn_linker_func_ret_addr
	ret

old_preamble:
	movq (%rsp), %rdi
	movq 8(%rsp), %rsi
	add $8, %rsp
	add $8, %rsp
	call elf_loader_dynamic_linker
	push %rax
	ret