#ifndef VIOS_DICTIONARY_INCLUDED
#define VIOS_DICTIONARY_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include "vit.h"

// Short forms
#define vi_dict_create vi_dictionary_create
#define vi_dict_add vi_dictionary_add
#define vi_dict_get_value vi_dictionary_get_value
#define vi_dict_remove_key vi_dictionary_remove_key
#define vi_dict_has_key vi_dictionary_has_key

typedef struct {
	char *key;
	void *value;
	uint32_t hash;
	uint32_t modified;

	void *prev;
	void *next;
} vi_dictionary_item;

typedef struct {
	vi_dictionary_item *head;
	vi_dictionary_item *tail;

	uint32_t count;
} vi_dictionary;

typedef void (*vi_dictionary_for_each_callback)(char *, void *);

int vit_dictionary_test( int argc, char *argv[] );
void vit_dict_test_callback( char *key, void *value );

int vi_dictionary_create( vi_dictionary *d );
int vi_dictionary_destroy( vi_dictionary *d );
int vi_dictionary_add( vi_dictionary *d, char *key, void *value );
vi_dictionary_item *vi_dictionary_get_item( vi_dictionary *d, char *key );
void *vi_dictionary_get_value( vi_dictionary *d, char *key );
int vi_dictionary_remove_key( vi_dictionary *d, char *key );
bool vi_dictionary_has_key( vi_dictionary *d, char *key );
int vi_dictionary_for_each( vi_dictionary *d, vi_dictionary_for_each_callback callback );
void vi_dictionary_dump_diagnostic( vi_dictionary *d );

/************************************************/
/* String Varient                               */
/************************************************/
typedef void (*vi_dictionary_string_for_each_callback)(char *, char *);

int vi_dictionary_string_create( vi_dictionary *d );
int vi_dictionary_string_destroy( vi_dictionary *d );
int vi_dictionary_string_add( vi_dictionary *d, char *key, char *value );
vi_dictionary_item *vi_dictionary_string_get_item( vi_dictionary *d, char *key );
char *vi_dictionary_string_get_value( vi_dictionary *d, char *key );
int vi_dictionary_string_remove_key( vi_dictionary *d, char *key );
bool vi_dictionary_string_has_key( vi_dictionary *d, char *key );
int vi_dictionary_string_for_each( vi_dictionary *d, vi_dictionary_string_for_each_callback callback );

#ifdef __cplusplus
}
#endif
#endif