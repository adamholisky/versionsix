#if !defined(VIOS_DEVICE_INCLUDED)
#define VIOS_DEVICE_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

class Device {
	protected:
		uint8_t *buffer;
		char full_name[25];
		char file_name[25];
	public:
		uint8_t *get_buffer( void ) { return buffer; }
		void set_buffer( uint8_t *b ) { buffer = b; }

		Device( );
		~Device();

		virtual void open( void );
		void close( void );
		void read( void );
		void write( void );
};

void device_initalize( void );

#ifdef __cplusplus
}
#endif
#endif