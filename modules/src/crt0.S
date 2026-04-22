.section .text

.global _start
_start:
	movq $0, %rbp
	pushq %rbp
	pushq %rbp
	movq %rsp, %rbp

	call process_env_setup

	popq %rdi
	popq %rsi

	call main

	movl %eax, %edi
	call process_exit
.size _start, . - _start