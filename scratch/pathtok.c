#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/*
Test conditions:
	/
	/home
	/home/adam
	/home/adam/
*/

int main( void ) {
	//char pathname[] = "/usr/local/osdev/source/versionsix/build_support";
	//char pathname[] = "/usr/local/osdev/source/versionsix/build_support/";
	char pathname[] = "/";


	char path_elements[25][256];
	int current_path_ele_index = 0;

	int name_index = 0;
	int path_index = 0;

	bool keep_going_main = true;
	bool keep_going_secondary = true;
	char* c = pathname;
	int path_length = strlen( c );

	memset( path_elements, 0, 25 * 256 );

	do {
		do {
			if ( *c == '/' || *c == 0 || path_index == path_length ) {
				if ( path_index == 0 ) {
					path_elements[0][0] = '/';
				}
				
				if ( path_index == path_length ) {
					keep_going_main = false;
				}

				keep_going_secondary = false;
				current_path_ele_index++;
				name_index = 0;
			}
			else {
				path_elements[current_path_ele_index][name_index++] = *c++;
				path_index++;
			}
		} while ( keep_going_secondary );

		c++;
		path_index++;
		printf( "path_elements[%d] = %s\n", current_path_ele_index - 1, path_elements[current_path_ele_index - 1] );

		if ( path_index >= path_length ) {
			keep_going_main = false;
		}
		else {
			keep_going_secondary = true;
		}
	} while ( keep_going_main );

	for ( int i = 0; i < current_path_ele_index; i++ ) {
		printf( "PE[%d]:\t%s\n", i, path_elements[i] );
	}


	return 0;
}
