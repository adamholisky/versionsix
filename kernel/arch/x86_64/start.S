.code64
.extern kernel_main
.global kernel_stack
.global _start
.type _start, @function

.section .data

	.skip 65536
kernel_stack:
	.skip 1024


.section .text

_start:
	mov $kernel_stack, %rsp
	
	push $0
	push $0

	mov %rsp, %rbp

	call kernel_main