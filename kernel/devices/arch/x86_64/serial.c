#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "bootstrap.h"
#include "serial.h"

uint32_t default_port;
uint32_t buffer_add_loc;
uint32_t buffer_read_loc;
char *data_buff;
uint32_t data_buff_loc;
bool data_is_being_read;
int32_t data_buffer_task;
bool data_ready;
bool serial_cr_recvd;

void serial_initalize(void)
{
	data_buff = NULL;
	data_buff_loc = 0;
	data_is_being_read = false;
	data_ready = false;
	serial_cr_recvd = false;

	// DO NOT PUT KLOG FUNCTIONS HERE
	serial_setup_port(COM1);
	serial_setup_port(COM4);

	default_port = COM1;
}

void serial_setup_port(uint32_t port)
{
	outportb(port + 1, 0x00); // Disable all interrupts
	outportb(port + 3, 0x80); // Enable DLAB (set baud rate divisor)
	outportb(port + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
	outportb(port + 1, 0x00); //                  (hi byte)
	outportb(port + 3, 0x03); // 8 bits, no parity, one stop bit
	outportb(port + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
	outportb(port + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

void serial_enable_interrupts( void ) {
	outportb(COM1 + 1, 0x01);
	outportb(COM2 + 1, 0x01);
	outportb(COM3 + 1, 0x01);
}

void serial_set_default_port(uint32_t port)
{
	default_port = port;
}

void serial_write_port(char c, uint32_t port)
{
	if (port == serial_use_default_port)
	{
		port = default_port;
	}

	// Make sure the transmit queue is empty
	while((inportb(port + 5) & 0x20) == 0) {
		;
	}

	outportb(port, c);
}

char serial_read_port(uint32_t port)
{
	char c = '\0';

	if (port == serial_use_default_port)
	{
		port = default_port;
	}

	// Wait until we can get something
	while ((inportb(port + 5) & 1) == 0)
	{
		;
	}

	c = inportb( port );

	//debugf( "\n\n. %d -- %c\n", c, c );

	return c;
}