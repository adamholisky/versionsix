#ifndef VIOS_DEBUG_INCLUDED
#define VIOS_DEBUG_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#define LOG_INFO 0      // General info
#define LOG_DEBUG 1     // Debug output, very verbose
#define LOG_ERROR 2     // General error, kernel can continue
#define LOG_PANIC 3     // Kernel panic, exec halted or will be halted

static const char * bit_array[16] = {
    [ 0] = "0000", [ 1] = "0001", [ 2] = "0010", [ 3] = "0011",
    [ 4] = "0100", [ 5] = "0101", [ 6] = "0110", [ 7] = "0111",
    [ 8] = "1000", [ 9] = "1001", [10] = "1010", [11] = "1011",
    [12] = "1100", [13] = "1101", [14] = "1110", [15] = "1111",
};

#define debugf( ... ) debugf_stage2( "[%s:%d] ", __FUNCTION__, __LINE__ ); debugf_stage2( __VA_ARGS__ )
#define debugf_raw( ... ) debugf_stage2( __VA_ARGS__ )
#define debugf_val( v ) debugf( "" #v " = 0x%llX (%d)\n", v, v )
#define df( ... ) debugf( __VA_ARGS__ )
#define dfv( v ) debugf_val( v )
#define dpf( ... ) debugf( __VA_ARGS__ ); printf( __VA_ARGS__ );

#define klog( level, ... ) klog_stage2( level, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__ )

#define log_entry_enter() debugf( "Enter\n" )
#define log_entry_exit() debugf( "Exit\n" );

#define dbA() debugf( "\n----------> A\n" )
#define dbB() debugf( "\n----------> B\n" )
#define dbC() debugf( "\n----------> C\n" )
#define dbD() debugf( "\n----------> D\n" )
#define dbE() debugf( "\n----------> E\n" )
#define db1() debugf( "1\n" )
#define db2() debugf( "2\n" )
#define db3() debugf( "3\n" )
#define db4() debugf( "4\n" )
#define db5() debugf( "5\n" )

#define dA dbA();
#define dB dbB();
#define dC dbC();
#define dD dbD();
#define dE dbE();

#define debugf_bit_array( x ) debugf_raw( "flags: %s %s %s %s %s %s %s %s -- 0x%08X\n", \
        bit_array[ 0xF & (x >> 28) ], \
        bit_array[ 0xF & (x >> 24) ], \
        bit_array[ 0xF & (x >> 20) ], \
        bit_array[ 0xF & (x >> 16) ], \
        bit_array[ 0xF & (x >> 12) ],\
        bit_array[ 0xF & (x >> 8) ], \
        bit_array[ 0xF & (x >> 4) ], \
        bit_array[ 0xF & x ], \
        x ); \
        debugf_raw( "       31   27   23   19   15   11   7    3\n" )

void debugf_stage2( char * message, ... );
void do_divide_by_zero( void );
void kdebug_peek_at( uint64_t addr );
char * kdebug_peek_at_n( uint64_t addr, int n );
char peek_char( char c );
char * kernel_symbols_get_function_at( uint64_t addr );
void klog_stage2( int level, char *file_name, char *function_name, int line_number, char * message, ... );
void klog_crash( uint64_t addr, void *p );

#ifdef __cplusplus
}
#endif
#endif