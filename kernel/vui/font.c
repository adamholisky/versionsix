#include "vit.h"
#include "vui/vui.h"
#include "vui/font.h"

uint16_t font_id_top;

vui_font fonts;
vui_font *main_font;

/**
 * @brief Initalize the font system
 * 
 */
void vui_font_initalize( void ) {
	fonts.next = NULL;	
	font_id_top = 0;
}

/**
 * @brief Loads the provided font
 * 
 * @param type Type of font: VUI_FONT_TYPE_
 * @param name Display and search name of the font
 * @param font_path Path to the font on the file system
 */
void vui_font_load( uint8_t type, char *name, char *font_path ) {
	vui_font *font_data_to_use = NULL;
	bool found = false;

	font_data_to_use = &fonts;

	if( font_id_top != 0 ) {
		font_data_to_use = &fonts;
		do {
			if( font_data_to_use->next == NULL ) {
				font_data_to_use->next = vmalloc( sizeof(vui_font) );
				font_data_to_use = font_data_to_use->next;

				memset( font_data_to_use, 0, sizeof(vui_font) );
				font_data_to_use->info.id = font_id_top++;
				strcpy( font_data_to_use->info.name, name );
				strcpy( font_data_to_use->info.path, font_path );

				found = true;
			} else {
				font_data_to_use = font_data_to_use->next;
			}
		} while( font_data_to_use != NULL && !found );
	} else {
		memset( font_data_to_use, 0, sizeof(vui_font) );
		font_data_to_use->info.id = font_id_top++;
		strcpy( font_data_to_use->info.name, name );
		strcpy( font_data_to_use->info.path, font_path );
		main_font = font_data_to_use;
	}

	switch( type ) {
		case VUI_FONT_TYPE_PSF:
			vui_font_load_psf( font_data_to_use );
			break;
		default:
			vdf( "Font Load: Unknown type. Got: %d.\n", type );
	}
}

/**
 * @brief Returns the font matching the provided name, otherwise NULL
 * 
 * @param name Name of the font that was previously loaded
 * @return vui_font* Pointer to the font structure, NULL on failure
 */
vui_font *vui_font_get_font( char *name ) {
	vui_font *font = &fonts;

	do {
		if( strcmp(font->info.name, name) == 0 ) {
			return font;
		}

		font = font->next;
	} while( font != NULL );

	return font;
}

/**
 * @brief Returns the active font's bitmap
 * 
 * @return font_bitmap* pointer to the bitmap array for the given font, NULL if none were loaded 
 */
vui_font *vui_font_get_main_font( void ) {
    return main_font;
}

void vui_font_get_bounding_box( char *text, vui_font *font, uint16_t *width, uint16_t *height ) {
	uint16_t num_chars = strlen(text);
	vui_font *f = ( font == NULL ? main_font : font );

	*width = num_chars * f->info.width;
	*height = f->info.height;
}

/**
 * @brief Loads the given PSF font, sets it as the primary font.
 * 
 * @param font_path Path to font on disk
 * @return true Font was loaded successfully
 * @return false Font failed to load
 */
#undef KDEBUG_FONT_LOAD_PSF
bool vui_font_load_psf( vui_font *font ) {

	#ifdef VI_ENV_OS
	vfs_stat_data stats;
	uint32_t file_size;

	int stat_error = vfs_stat( vfs_lookup_inode(font->info.path), &stats );
	if( stat_error != VFS_ERROR_NONE ) {
		debugf( "Error: %d\n", stat_error );
		return false;
	}

	file_size = stats.size;

	uint8_t *data = vmalloc( file_size );
	int read_err = vfs_read( vfs_lookup_inode(font->info.path), data, file_size, 0 );
	if( read_err < VFS_ERROR_NONE ) {
		debugf( "Error when reading: %d\n", read_err );
		return false;
	}
	#else
	FILE *f = fopen( font->info.path, "r" );

	if( f == NULL ) {
		vdf( "Cannot open %s, f returned NULL.\n", font->info.path );
		return false;
	}

	struct stat file_meta;
	stat( font->info.path, &file_meta );

	uint32_t file_size = file_meta.st_size;

	uint8_t *data = vmalloc( file_size );

	if( fread( data, file_size, 1, f ) != 1 ) {
		vdf( "Read failed on %s, fread returned not 1.\n", font->info.path );
		return false;
	}
	#endif

	data[ file_size ] = 0;

	psf_font *header = (psf_font *)data;

	uint16_t header_size = 0;

	if( (uint16_t)header->magic == PSF1_FONT_MAGIC ) {
		psf1_header *v1_header = (psf1_header *)header;
		font->info.num_glyphs = v1_header->fontMode & 0x1 ? 512 : 256;
		font->info.height = v1_header->characterSize;
		font->info.width = 8;

		#ifdef KDEBUG_FONT_LOAD_PSF
		vdf( "Header type 1!\n" );
		vdf( "Number glyphs: %d\n", font->info.num_glyphs );
		vdf( "Height: %d\n", font->info.height );
		vdf( "Width: %d\n", font->info.width );
		#endif

		header_size = sizeof( psf1_header );
	} else {
		#ifdef KDEBUG_FONT_LOAD_PSF
		vdf( "Header type 2!\n" );
		vdf( "Number glyphs: %d\n", header->numglyph );
		vdf( "Bytes per glyph: %d\n", header->bytesperglyph );
		vdf( "Flags: %X\n", header->flags );
		vdf( "Height: %d\n", header->height );
		vdf( "Width: %d\n", header->width );
		vdf( "uint16: %d\n", sizeof(uint16_t) );
		vdf( "Header size: 0x%X\n", header->headersize );
		#endif

		font->info.height = header->height;
		font->info.width = header->width;
		font->info.num_glyphs = header->numglyph;

		header_size = sizeof( psf_font );
	}

	font->bitmaps = vmalloc( sizeof(font_bitmap) * font->info.num_glyphs);

	

	#ifdef KDEBUG_FONT_LOAD_PSF
	vdf( "Data Start: 0x%016llx\n", data );
    #endif

	if( font->info.width <= 8 ) {
		uint8_t *start = (uint8_t *)(data + header_size);

		for( int i = 0; i < font->info.num_glyphs; i++ ) {
			font->bitmaps[i].num = i;

			for( int j = 0; j < font->info.height; j++ ) {
				font->bitmaps[i].pixel_row[j] = (*start << 8);
				
				*start++;
			}
		}
	} else {
		uint16_t *start = (uint16_t *)(data + header_size);
		for( int i = 0; i < font->info.num_glyphs; i++ ) {
			font->bitmaps[i].num = i;

			for( int j = 0; j < font->info.height; j++ ) {
				font->bitmaps[i].pixel_row[j] = (*start << 8) | (*start  >> 8);
				
				*start++;
			}
		}
	}
	
    return true;
}

/**
 * @brief Loads the provided bdf font, sets as primary font
 * 
 * @param font_path Path to the font on disk
 * @return true Font was loaded successfully
 * @return false Loading the font failed.
 * 
 * Test font: "/usr/share/fonts/gomme10x20n.bdf"
 */
/* bool font_load_bdf( char *font_path ) {
	vfs_stat_data stats;

	int stat_error = vfs_stat( vfs_lookup_inode(font_path), &stats );
	if( stat_error != VFS_ERROR_NONE ) {
		debugf( "Error: %d\n", stat_error );
		return false;
	}

	uint8_t *data = vfs_malloc( stats.size );
	int read_err = vfs_read( vfs_lookup_inode(font_path), data, stats.size, 0 );
	if( read_err < VFS_ERROR_NONE ) {
		debugf( "Error when reading: %d\n", read_err );
		return false;
	}

	data[ stats.size ] = 0;

	bool keep_going = true;
	bool in_char = false;
	uint16_t current_char = 0;
	bool in_bitmap = false;
	int bitmap_line = 0;
	int j = 0;

	do {
		char line[250];
		
		memset( line, 0, 250 );

		for( int i = 0; i < 250; i++ ) {
			if( *data == '\n' ) {
				data++;
				i = 250;
			} else {
				line[i] = *data++;
			}
		}

		if( in_char ) {
			if( in_bitmap ) {
				if( strncmp(line, "ENDCHAR", sizeof("ENDCHAR") - 1) == 0 ) {
					in_char = false;
					in_bitmap = false;
					bitmap_line = 0;
					j++;
				} else {
					main_font_bitmaps[j].pixel_row[bitmap_line] = strtol(line, NULL, 16);

					bitmap_line++;
				}
			} else {
				if( strncmp(line, "ENCODING ", sizeof("ENCODING ") - 1) == 0 ) {
					char *encoding_line = line;
					encoding_line = encoding_line + sizeof("ENCODING ") - 1;
					current_char = atoi(encoding_line);
					main_font_bitmaps[j].num = current_char;
					//debugf( "Found 0x%X (%d) '%c'\n", current_char, current_char, current_char );
				} else if( strncmp(line, "BITMAP", sizeof("BITMAP") - 1) == 0 ) {
					in_bitmap = true;
				}
			}
		} else {
			if( strncmp(line, "CHARS ", 6) == 0 ) {
				char *chars_line = line;
				chars_line = chars_line + 6;
				uint16_t num_chars = atoi(chars_line);
				main_font_bitmaps = kmalloc( sizeof(font_bitmap) * num_chars );
				//debugf( "Num Chars: %d\n", num_chars );
			} else if( strncmp(line, "STARTCHAR ", sizeof("STARTCHAR ") - 1) == 0 ) {
				in_char = true;
			}
		}

		if( *data == 0 ) {
			keep_going = false;
		}
	} while( keep_going );

	//debugf( "added %d chars\n", j );

    return true;
}
 */