#include <stdio.h>
#include <string.h>

void klog_string_escape_json( char *unescaped_str, char *escaped_str );

const char test_str[] = "Appended mount \ point. fs_type=ASVFS  mount_path=\"/\"  drive_id=0";

int main( int argc, char *argv[] ) {
	char message[255];
	char escaped_str[255];

	strcpy( message, test_str );

	printf( "Test String: len=%ld  text=%s\n", strlen(message), message );
	
	memset( escaped_str, 0, 255 );
	klog_string_escape_json( message, escaped_str );

	printf( "Escp String: len=%ld  text=%s\n", strlen(escaped_str), escaped_str );

	return 0;
}

void klog_string_escape_json( char *unescaped_str, char *escaped_str ) {
	char *s = unescaped_str;
	int esc_index = 0;

	while(*s) {
		switch( *s ) {
			case '"':
				escaped_str[esc_index] = '\\';
				esc_index++;
				escaped_str[esc_index] = '"';
			default:
				escaped_str[esc_index] = *s;
		}

		esc_index++;
		s++;
	}
}