#ifndef VIOS_TASK_INCLUDED
#define VIOS_TASK_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

class Task {
	protected:
		char full_name[25];
		char file_name[25];
	public:
		Task( );
		~Task();

		virtual void open( void );
		void close( void );
		void read( void );
		void write( void );
};

void task_initalize( void );

#ifdef __cplusplus
}
#endif
#endif