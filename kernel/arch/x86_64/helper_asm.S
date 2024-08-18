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

.global setup_kernel_stack
setup_kernel_stack:
	pop %r11
		mov %rdi, %rsp

		push $0
		push %rsi
		push %rbx
		mov %rsp, %rbp
		sub $0x48, %rsp
	push %r11
	xor %r11, %r11

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