#include <kernel_common.h>
#include <interrupt.h>
#include <timer.h>
#include <ksymbols.h>
#include <stacktrace.h>
#include <task.h>

interrupt_descriptor_table main_idtr;
interrupt_gate_descriptor IDT[256];
irq_handler irq_handlers[256];
uint8_t pic1_irq_mask;
uint8_t pic2_irq_mask;

void interrupt_initalize( void ) {
	// First: Setup the PIC
	uint8_t primary_data;
	uint8_t secondary_data;

	primary_data = inportb( PIC_PRIMARY_DATA );
	secondary_data = inportb( PIC_SECONDARY_DATA );

	outportb( PIC_PRIMARY_COMMAND, PIC_EOI );

	outportb( PIC_PRIMARY_COMMAND, ICW1_INIT + ICW1_ICW4 );
	outportb( PIC_SECONDARY_COMMAND, ICW1_INIT + ICW1_ICW4 );

	outportb( PIC_PRIMARY_DATA, PIC_PRIMARY_OFFSET );
	outportb( PIC_SECONDARY_DATA, PIC_SECONDARY_OFFSET );

	outportb( PIC_PRIMARY_DATA, 0x04 );
	outportb( PIC_SECONDARY_DATA, 0x02 );

	outportb( PIC_PRIMARY_DATA, ICW4_8086 );
	outportb( PIC_SECONDARY_DATA, ICW4_8086 );

	outportb( PIC_PRIMARY_DATA, primary_data );
	outportb( PIC_SECONDARY_DATA, secondary_data );

	// Second: Setup exception and IRQ handlers
	interrupt_setup_exception_handler( 0, (uint64_t)isr_0 );
	interrupt_setup_exception_handler( 1, (uint64_t)isr_1 );
	interrupt_setup_exception_handler( 2, (uint64_t)isr_2 );
	interrupt_setup_exception_handler( 3, (uint64_t)isr_3 );
	interrupt_setup_exception_handler( 4, (uint64_t)isr_4 );
	interrupt_setup_exception_handler( 5, (uint64_t)isr_5 );
	interrupt_setup_exception_handler( 6, (uint64_t)isr_6 );
	interrupt_setup_exception_handler( 7, (uint64_t)isr_7 );
	interrupt_setup_exception_handler( 8, (uint64_t)isr_8 );
	interrupt_setup_exception_handler( 9, (uint64_t)isr_9 );
	interrupt_setup_exception_handler( 10, (uint64_t)isr_10 );
	interrupt_setup_exception_handler( 11, (uint64_t)isr_11 );
	interrupt_setup_exception_handler( 12, (uint64_t)isr_12 );
	interrupt_setup_exception_handler( 13, (uint64_t)isr_13 );
	interrupt_setup_exception_handler( 14, (uint64_t)isr_14 );
	interrupt_setup_exception_handler( 15, (uint64_t)isr_15 );
	interrupt_setup_exception_handler( 16, (uint64_t)isr_16 );
	interrupt_setup_exception_handler( 17, (uint64_t)isr_17 );
	interrupt_setup_exception_handler( 18, (uint64_t)isr_18 );
	interrupt_setup_exception_handler( 19, (uint64_t)isr_19 );
	interrupt_setup_exception_handler( 20, (uint64_t)isr_20 );
	
	interrupt_setup_exception_handler( 32, (uint64_t)isr_32 );
	interrupt_setup_exception_handler( 33, (uint64_t)isr_33 );
	interrupt_setup_exception_handler( 34, (uint64_t)isr_34 );
	interrupt_setup_exception_handler( 35, (uint64_t)isr_35 );
	interrupt_setup_exception_handler( 36, (uint64_t)isr_36 );
	interrupt_setup_exception_handler( 37, (uint64_t)isr_37 );
	interrupt_setup_exception_handler( 38, (uint64_t)isr_38 );
	interrupt_setup_exception_handler( 39, (uint64_t)isr_39 );
	interrupt_setup_exception_handler( 40, (uint64_t)isr_40 );
	interrupt_setup_exception_handler( 41, (uint64_t)isr_41 );
	interrupt_setup_exception_handler( 42, (uint64_t)isr_42 );
	interrupt_setup_exception_handler( 43, (uint64_t)isr_43 );
	interrupt_setup_exception_handler( 44, (uint64_t)isr_44 );
	interrupt_setup_exception_handler( 45, (uint64_t)isr_45 );
	interrupt_setup_exception_handler( 46, (uint64_t)isr_46 );
	interrupt_setup_exception_handler( 47, (uint64_t)isr_47 );
	interrupt_setup_exception_handler( 254, (uint64_t)isr_254 );


	for( int i = 0; i < 255; i++ ) {
		irq_handlers[i].in_use = false;
		irq_handlers[i].handler = NULL;
	}

	// Third: Setup and load the IDT
	main_idtr.limit = sizeof(IDT) - 1;
	main_idtr.base = IDT;

	__asm__ volatile("lidt %0;"
                     :
                     :"m"(main_idtr)
                     :"cc");

	// Fourth: Mask other interrupts
	pic1_irq_mask = 0x00;
	pic2_irq_mask = 0x00;
	outportb( PIC_PRIMARY_DATA, pic1_irq_mask );
	outportb( PIC_SECONDARY_DATA, pic2_irq_mask );

	// Fifth: Enable interrupts
	__asm__ volatile("sti");
}

#undef INTERRUPT_SETUP_EXCEPTION_HANDLER_DEBUG
void interrupt_setup_exception_handler( int num, uint64_t handler ) {
	IDT[num].selector = 0x28;
	IDT[num].ist = 0;
	IDT[num].type_attributes = 0x8f;
	IDT[num].offset_1 = handler & 0xFFFF;
	IDT[num].offset_2 = (handler >> 16) & 0xFFFF;
	IDT[num].offset_3 = (handler >> 32) & 0xFFFFFFFF;

	#ifdef INTERRUPT_SETUP_EXCEPTION_HANDLER_DEBUG
	printf( "isr_9: 0x%p\n", isr_0 );
	printf( "hando: 0x%p\n", handler );
	printf( "off 1: 0x%08X\n", IDT[0].offset_1 );
	printf( "off 2: 0x%08X\n", IDT[0].offset_2 );
	printf( "off 3: 0x%08X\n", IDT[0].offset_3 );
	#endif
}

#undef DEBUG_INTERRUPT_HANDLER_STAGE_2
void interrupt_handler_stage_2( registers **_reg ) {
	registers *reg = *_reg;

	uint64_t interrupt_number_at_entry = reg->interrupt_no;

	#ifdef DEBUG_INTERRUPT_HANDLER_STAGE_2
	if( reg->interrupt_no != 0x20 ) {
		debugf( "Interrupt( num = 0x%X )\n", reg->interrupt_no );
	}
	#endif

	if( reg->interrupt_no < 21 ) {
		uint64_t *stack = (uint64_t *)reg->rsp;
		uint16_t current_task_id = task_get_current_task_id();

		debugf_raw( "================================================================================\n" );
		debugf_raw( "Exception %d: %s \n", reg->interrupt_no, intel_exceptions[reg->interrupt_no] );
		debugf_raw( "    task: %d\n", current_task_id );
		debugf_raw( "    rip:  0x%016llX (%s)\n", reg->rip, kernel_symbols_get_function_name_at(reg->rip) );
		debugf_raw( "    rax:  0x%016llX  rbx:  0x%016llX  rcx:  0x%016llX\n", reg->rax, reg->rbx, reg->rcx );
		debugf_raw( "    rdx:  0x%016llX  rsi:  0x%016llX  rdi:  0x%016llX\n", reg->rdx, reg->rsi, reg->rdi );
		debugf_raw( "    rsp:  0x%016llX  rbp:  0x%016llX  cr0:  0x%016llX \n", reg->rsp, reg->rbp, get_cr0() );
		debugf_raw( "    cr2:  0x%016llX  cr3:  0x%016llX  cr4:  0x%016llX\n", get_cr2(), get_cr3(), get_cr4() );
		debugf_raw( "    cs:   0x%04X  num:  0x%08X  err:  0x%08X  flag: 0x%08X\n", reg->cs, reg->interrupt_no, reg->error_no, reg->rflags);
		debugf_raw( "\n" );
		debugf_raw( "    Stack Trace:\n" );
		
		stacktrace_out_for_rbp( reg->rbp, false, true, 4 );

		debugf_raw( "================================================================================\n" );

		if( current_task_id == 0 ) {
			debugf_raw( "\nKernel task generated exception. Halting.\n" );

			while( 1 ) {
				__asm__ volatile( "nop" );
			}
		} else {
			task *t = get_task_data( current_task_id );
			t->exit_code = 1;
			debugf_raw( "Ending task %d, switching to parent id %d\n", current_task_id, t->parent_task_id );
			task_set_task_status( current_task_id, TASK_STATUS_DEAD );
			task_set_yield_to_next( t->parent_task_id );
			task_sched_yield( _reg );
		}
	} else {
		if( irq_handlers[reg->interrupt_no - 0x20].in_use == true ) {
			irq_handler_func handler_func = (irq_handler_func)(irq_handlers[reg->interrupt_no - 0x20].handler);

			handler_func( _reg );
		} else {
			debugf_raw( "Unknown interrupt: %d\n", reg->interrupt_no - 0x20 );

			/* while( 1 ) {
				__asm__ volatile( "nop" );
			} */
		}
	}

	if( interrupt_number_at_entry - 0x20 >= 8 ) {
			outportb( PIC_SECONDARY_COMMAND, PIC_EOI );
	} 

	outportb( PIC_PRIMARY_COMMAND, PIC_EOI );
}

#define INTERRUPT_ADD_IRQ_HANDLER_DEBUG
void interrupt_add_irq_handler( uint8_t irq_num, irq_handler_func handler_func ) {
	if( irq_handlers[irq_num].in_use == true ) {
		debugf( "IRQ handler already assisgned. IRQ Num: %d, handler address: %p\n", irq_num, handler_func );
	}

	irq_handlers[irq_num].in_use = true;
	irq_handlers[irq_num].handler = handler_func;

	/* if( irq_num < 8 ) {
		uint8_t irq = irq_num & (1 << irq_num);
		dfv( irq );
		pic1_irq_mask = pic1_irq_mask ^ (1 << irq);
		//outportb( PIC_PRIMARY_DATA, pic1_irq_mask );
	} else {
		//uint8_t irq = irq_num & (1 << (irq_num - 8));
		//dfv( irq );
		pic2_irq_mask = pic2_irq_mask & ~(1 << (irq_num - 8));
		dfv( pic2_irq_mask );
		//outportb( PIC_SECONDARY_DATA, pic2_irq_mask );
	} */

	debugf( "IRQ Handler Assigned. IRQ Num: %d, handler address: %p\n", irq_num, handler_func );
}

void interrupts_disable( void ) {
	__asm__ __volatile__ ( "cli" );
}

void interrupts_enable( void ) {
	__asm__ __volatile__ ( "sti" );
}