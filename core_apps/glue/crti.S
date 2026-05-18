.section .init 
.global _init
_init:
	push %rbp
	moveq %rsp, %rbp

.section .fini
.global _fini
_fini:
	push %rbp
	moveq %rsp, %rbp
	