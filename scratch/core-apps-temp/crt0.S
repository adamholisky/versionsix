.section .text

.global _start
_start:
	movq $0, %rbp
	pushq %rbp
	pushq %rbp
	movq %rsp, %rbp

	pushq %rsi
	pushq %rdi

	call avs_libc_init

	call _init	// should be the last thing called, right before main. expects libc setup.
	
	popq %rdi
	popq %rsi
	call main
	push %eax

	call _fini
	call avs_libc_end

	pop $eax
	movl %eax, %edi
	call exit	// syscall, no return
.size _start, . - _start