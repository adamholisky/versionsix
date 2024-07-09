#ifndef VIOS_INTERRUPT_INCLUDED
#define VIOS_INTERRUPT_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define PIC_PRIMARY_COMMAND	0x20
#define PIC_PRIMARY_DATA	0x21
#define PIC_SECONDARY_COMMAND 0xA0
#define PIC_SECONDARY_DATA	0xA1
#define PIC_EOI				0x20
#define ICW1_INIT           0x10
#define ICW1_EDGE           0x08 
#define ICW1_SINGLE         0x02
#define	ICW1_ICW4           0x01
#define ICW4_SFNM           0x10
#define ICW4_BUFFER         0x08
#define ICW4_MASTER         0x04
#define ICW4_AEOI           0x02
#define ICW4_8086           0x01

#define PIC_PRIMARY_OFFSET 0x20
#define PIC_SECONDARY_OFFSET 0x28

static const char * intel_exceptions[21] = {
	[0] = "Divide by Zero",
	[1] = "Debug",
	[2] = "NMI",
	[3] = "Breakpoint",
	[4] = "Overflow",
	[5] = "Bound Range Exceeded",
	[6] = "Invalid Opcode",
	[7] = "Device Not Available",
	[8] = "Double Fault",
	[9] = "Coprocessor Segment Overrun",
	[10] = "Invalid TSS",
	[11] = "Segment Not Present",
	[12] = "Stack-Segment Fault",
	[13] = "General Protection Fault",
	[14] = "Page Fault",
	[15] = "Reserved",
	[16] = "x87 Floating-Point Exception",
	[17] = "Alignment Check",
	[18] = "Machine Check",
	[19] = "SIMD Floating-Point Exception",
	[20] = "Virtualization Exception"
};

typedef struct {
	uint16_t offset_1;
	uint16_t selector;
	uint8_t  ist;
	uint8_t  type_attributes;
	uint16_t offset_2;
	uint32_t offset_3;
	uint32_t zero;
}  __attribute__ ((packed)) interrupt_gate_descriptor;

typedef struct {
	 uint16_t limit;
	 interrupt_gate_descriptor *base;
} __attribute__ ((packed)) interrupt_descriptor_table;

typedef struct {
	uint64_t    rax;
	uint64_t    rbx;
	uint64_t    rcx;
	uint64_t    rdx;

	uint64_t    rsi;
	uint64_t    rdi;
	uint64_t    rbp;

	uint64_t    r8;
	uint64_t    r9;
	uint64_t    r10;
	uint64_t    r11;
	uint64_t    r12;
	uint64_t    r13;
	uint64_t    r14;
	uint64_t    r15;

	uint64_t    interrupt_no;
	uint64_t    error_no;

	uint64_t    rip;
	uint64_t    cs;
	uint64_t    rflags;

	uint64_t    rsp;
	uint64_t    ss;
} __attribute__ ((packed)) registers;

struct stackframe {
	struct stackframe *rbp;
	uint64_t rip;
};

typedef void (*irq_handler_func)( registers **_context );

typedef struct {
	bool in_use;
	irq_handler_func handler;
} irq_handler;

extern void isr_0( void );
extern void isr_1( void );
extern void isr_2( void );
extern void isr_3( void );
extern void isr_4( void );
extern void isr_5( void );
extern void isr_6( void );
extern void isr_7( void );
extern void isr_8( void );
extern void isr_9( void );
extern void isr_10( void );
extern void isr_11( void );
extern void isr_12( void );
extern void isr_13( void );
extern void isr_14( void );
extern void isr_15( void );
extern void isr_16( void );
extern void isr_17( void );
extern void isr_18( void );
extern void isr_19( void );
extern void isr_20( void );

extern void isr_32( void );
extern void isr_33( void );
extern void isr_34( void );
extern void isr_35( void );
extern void isr_36( void );
extern void isr_37( void );
extern void isr_38( void );
extern void isr_39( void );
extern void isr_40( void );
extern void isr_41( void );
extern void isr_42( void );
extern void isr_43( void );
extern void isr_44( void );
extern void isr_45( void );
extern void isr_46( void );
extern void isr_47( void );

extern void isr_254( void );


extern uint64_t get_cr0( void );
extern uint64_t get_cr2( void );
extern uint64_t get_cr3( void );
extern uint64_t get_cr4( void );

void interrupt_initalize( void );
void interrupt_handler_stage_2( registers **_reg );
void interrupt_setup_exception_handler( int num, uint64_t handler );
void interrupt_add_irq_handler( uint8_t irq_num, irq_handler_func handler_func );
void interrupts_disable( void );
void interrupts_enable( void );

#ifdef __cplusplus
}
#endif
#endif