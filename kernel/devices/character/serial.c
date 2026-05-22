#include <kernel_common.h>
#include <device.h>
#include <devices/serial.h>
#include <kmemory.h>
#include <wait_queue.h>

device serial3;
device serial4;

wait_queue *wq_serial3;

int serial3_buffer_w_index = 0;
int serial3_buffer_r_index = 0;
uint8_t serial3_buffer[50];

typedef struct
{
	registers **context;
	int fd;
	char *buff;
	size_t count;
} serial3_wq_data;

/*
 * /dev/serial3
 */

void device_register_serial3(void)
{
	memset(&serial3, 0, sizeof(device));

	strcpy(serial3.major_id, "serial");
	strcpy(serial3.minor_id, "3");

	serial3.close = serial3_close;
	serial3.open = serial3_open;
	serial3.read = serial3_read;
	serial3.write = serial3_write;
	serial3.interrupt_handler = serial3_interrupt_handler;

	debugf("Registered.\n");

	wq_serial3 = wq_new("serial3", serial3_wq_ready);

	device_register(&serial3);
}

void serial3_close(inode_id id)
{
	// Intentionally blank
}

void serial3_open(inode_id id)
{
	// Intentionally blank
}

uint8_t serial3_read(inode_id id, uint8_t *buff, uint64_t count, uint64_t offset)
{
	// return serial_read_port( COM3 );
	uint8_t c = 0;

	if (serial3_buffer_r_index < serial3_buffer_w_index)
	{
		c = serial3_buffer[serial3_buffer_r_index++];

		if (serial3_buffer_r_index == 50)
		{
			serial3_buffer_r_index = 0;
		}
	}
	else
	{
		serial3_wq_data *wqd = kmalloc(sizeof(serial3_wq_data));

		wqd->fd = id;
		wqd->buff = buff;
		wqd->count = count;

		wq_add(wq_serial3, process_get_current_proc_id(), wqd);
	}

	return c;
}

void serial3_write(inode_id id, void *buff, size_t count, size_t offset)
{
	char *char_buff = (char *)buff;
	char *char_buff_end = (char *)buff + count;

	while (char_buff != char_buff_end)
	{
		serial_write_port(*char_buff, COM3);
		char_buff++;
	}
}

void serial3_interrupt_handler(registers **reg)
{
	serial3_buffer[serial3_buffer_w_index++] = serial_read_port(COM3);

	if (serial3_buffer_w_index == 50)
	{
		serial3_buffer_w_index = 0;
	}

	wq_make_ready(wq_serial3);
	wq_call_next_ready(wq_serial3);
}

uint8_t serial3_wq_ready(wait_queue *queue, pid_t pid, void *wq_data)
{
	char c = ' ';
	serial3_wq_data *wqd = (serial3_wq_data *)wq_data;

	process_data *p = process_get_data_from_pid(pid);

	c = serial3_buffer[serial3_buffer_r_index++];

	if (serial3_buffer_r_index == 50)
	{
		serial3_buffer_r_index = 0;
	}

	*wqd->buff = c;
	p->status = PROCESS_STATUS_INACTIVE;

	klog(LOG_INFO, "Got a c: %d", c);

	return c;
}

/*
 * /dev/serial4
 */

device *device_register_serial4(void)
{
	memset(&serial4, 0, sizeof(device));

	strcpy(serial4.major_id, "serial");
	strcpy(serial4.minor_id, "4");

	serial4.close = serial4_close;
	serial4.open = serial4_open;
	serial4.read = serial4_read;
	serial4.write = serial4_write;

	device_register(&serial4);
}

void serial4_open(inode_id id)
{
	// Intentionally blank
}

void serial4_close(inode_id id)
{
	// Intentionally blank
}

uint8_t serial4_read(inode_id id, uint8_t *buff, uint64_t count, uint64_t offset)
{
	return 0;
}

void serial4_write(inode_id id, void *buff, size_t count, size_t offset)
{
	char *char_buff = (char *)buff;
	char *char_buff_end = (char *)buff + count;

	while (char_buff != char_buff_end)
	{
		serial_write_port(*char_buff, COM4);
		char_buff++;
	}
}

/****************************
 * Generic Serial Functions *
 * for early OS setup       *
 ****************************/

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
	serial_setup_port(COM3);
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

void serial_enable_interrupts(void)
{
	// outportb(COM1 + 1, 0x01);
	// outportb(COM2 + 1, 0x01);
	outportb(COM3 + 1, 0x01);

	interrupt_add_irq_handler(4, serial3_interrupt_handler);
}

void serial_write_port(char c, uint32_t port)
{
	if (port == serial_use_default_port)
	{
		port = default_port;
	}

	// Make sure the transmit queue is empty
	while ((inportb(port + 5) & 0x20) == 0)
	{
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

	c = inportb(port);

	// debugf( "\n\n. %d -- %c\n", c, c );

	return c;
}