#include <stdint.h>
#include <stddef.h>
#include "syscall.h"
#include "file.h"
#include "serial.h"

size_t write( int fd, void *buff, size_t count ) {
    uint32_t com_port = COM1;

    if( fd == FD_STDERR ) {
        com_port = COM4;
    }
    
    char *char_buff = (char *)buff;
    char *char_buff_end = (char *)buff + count;
    
    while( char_buff != char_buff_end ) {
        serial_write_port( *char_buff, com_port );
        char_buff++;
    }
}