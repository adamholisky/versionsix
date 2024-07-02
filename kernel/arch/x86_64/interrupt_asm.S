.section .text

.extern interrupt_handler_stage_2

.type interrupt_handler_stage_1, @function
interrupt_handler_stage_1:
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
			mov %rsp, %rdi

			call interrupt_handler_stage_2
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

iretq

.macro isr_without_error_code num
.type isr_\num, @function
.global isr_\num
isr_\num:
	push $0
	push $\num
	jmp interrupt_handler_stage_1
.endm

.macro isr_with_error_code num
.global isr_\num
isr_\num:
	push $\num
	jmp interrupt_handler_stage_1
.endm

isr_without_error_code 0	# Divide 0
isr_without_error_code 1	# Debug
isr_without_error_code 2	# NMI
isr_without_error_code 3	# Break
isr_without_error_code 4	# Overflow
isr_without_error_code 5	# Range
isr_without_error_code 6	# Invalid Opcode
isr_without_error_code 7	# Device Not Available
isr_with_error_code 8		# Double Fault
isr_without_error_code 9	# N/A
isr_with_error_code 10		# Invalid TSS
isr_with_error_code 11		# Segment Not Present
isr_with_error_code 12		# Stack-Segment Fault
isr_with_error_code 13		# General Protection Fault
isr_with_error_code 14		# Page Fault
isr_without_error_code 15	# Reserved
isr_without_error_code 16	# Floating Point Exception
isr_with_error_code 17		# Alignment Check
isr_without_error_code 18	# Machine Check
isr_without_error_code 19	# SIMD Floating Point Exception
isr_without_error_code 20	# Virtulization Exception

isr_without_error_code 32	# Timer
isr_without_error_code 33
isr_without_error_code 34
isr_without_error_code 35   # Ethernet?
isr_without_error_code 36
isr_without_error_code 37
isr_without_error_code 38
isr_without_error_code 39
isr_without_error_code 40
isr_without_error_code 41
isr_without_error_code 42
isr_without_error_code 43
isr_without_error_code 44
isr_without_error_code 45
isr_without_error_code 46
isr_without_error_code 47

isr_without_error_code 254  # Syscall