#include "vit.h"
#include "vui/vui.h"
#include "vui/font.h"
#include "vui/schrift.h"

uint16_t font_id_top;

vui_font fonts;
vui_font *main_font;

uint16_t dos_code_page[256] = {
    0x0000,0x263A,0x263B,0x2665,0x2666,0x2663,0x2660,0x2022,0x25D8,0x25CB,0x25D9,0x2642,0x2640,0x266A,0x266B,0x263C,
    0x25BA,0x25C4,0x2195,0x203C,0x00B6,0x00A7,0x25AC,0x21A8,0x2191,0x2193,0x2192,0x2190,0x221F,0x2194,0x25B2,0x25BC,
    0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,0x0028,0x0029,0x002a,0x002b,0x002c,0x002d,0x002e,0x002f,
    0x0030,0x0031,0x0032,0x0033,0x0034,0x0035,0x0036,0x0037,0x0038,0x0039,0x003a,0x003b,0x003c,0x003d,0x003e,0x003f,
    0x0040,0x0041,0x0042,0x0043,0x0044,0x0045,0x0046,0x0047,0x0048,0x0049,0x004a,0x004b,0x004c,0x004d,0x004e,0x004f,
    0x0050,0x0051,0x0052,0x0053,0x0054,0x0055,0x0056,0x0057,0x0058,0x0059,0x005a,0x005b,0x005c,0x005d,0x005e,0x005f,
    0x0060,0x0061,0x0062,0x0063,0x0064,0x0065,0x0066,0x0067,0x0068,0x0069,0x006a,0x006b,0x006c,0x006d,0x006e,0x006f,
    0x0070,0x0071,0x0072,0x0073,0x0074,0x0075,0x0076,0x0077,0x0078,0x0079,0x007a,0x007b,0x007c,0x007d,0x007e,0x007f,
    0x00c7,0x00fc,0x00e9,0x00e2,0x00e4,0x00e0,0x00e5,0x00e7,0x00ea,0x00eb,0x00e8,0x00ef,0x00ee,0x00ec,0x00c4,0x00c5,
    0x00c9,0x00e6,0x00c6,0x00f4,0x00f6,0x00f2,0x00fb,0x00f9,0x00ff,0x00d6,0x00dc,0x00a2,0x00a3,0x00a5,0x20a7,0x0192,
    0x00e1,0x00ed,0x00f3,0x00fa,0x00f1,0x00d1,0x00aa,0x00ba,0x00bf,0x2310,0x00ac,0x00bd,0x00bc,0x00a1,0x00ab,0x00bb,
    0x2591,0x2592,0x2593,0x2502,0x2524,0x2561,0x2562,0x2556,0x2555,0x2563,0x2551,0x2557,0x255d,0x255c,0x255b,0x2510,
    0x2514,0x2534,0x252c,0x251c,0x2500,0x253c,0x255e,0x255f,0x255a,0x2554,0x2569,0x2566,0x2560,0x2550,0x256c,0x2567,
    0x2568,0x2564,0x2565,0x2559,0x2558,0x2552,0x2553,0x256b,0x256a,0x2518,0x250c,0x2588,0x2584,0x258c,0x2590,0x2580,
    0x03b1,0x00df,0x0393,0x03c0,0x03a3,0x03c3,0x00b5,0x03c4,0x03a6,0x0398,0x03a9,0x03b4,0x221e,0x03c6,0x03b5,0x2229,
    0x2261,0x00b1,0x2265,0x2264,0x2320,0x2321,0x00f7,0x2248,0x00b0,0x2219,0x00b7,0x221a,0x207f,0x00b2,0x25a0,0x00a0
};


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
		case VUI_FONT_TYPE_TTF:
			vui_font_load_ttf( font_data_to_use );
			break;
		default:
			vdf( "Font Load: Unknown type. Got: %d.\n", type );
	}

	if( type == VUI_FONT_TYPE_PSF ) {
		for( int i = 0; i < font_data_to_use->info.num_glyphs; i++ ) {
			vui_font_create_aa_mask( font_data_to_use, i );
		}
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

void vui_font_load_ttf( vui_font *font ) {
	#ifdef VI_ENV_OS
	vfs_stat_data stats;
	uint32_t file_size;

	int stat_error = vfs_stat( vfs_lookup_inode(font->info.path), &stats );
	if( stat_error != VFS_ERROR_NONE ) {
		debugf( "Error: %d\n", stat_error );
		return;
	}

	file_size = stats.size;

	uint8_t *data = vmalloc( file_size );
	int read_err = vfs_read( vfs_lookup_inode(font->info.path), data, file_size, 0 );
	if( read_err < VFS_ERROR_NONE ) {
		debugf( "Error when reading: %d\n", read_err );
		return;
	}
	#else
	FILE *f = fopen( font->info.path, "r" );

	if( f == NULL ) {
		vdf( "Cannot open %s, f returned NULL.\n", font->info.path );
		return;
	}

	struct stat file_meta;
	stat( font->info.path, &file_meta );

	uint32_t file_size = file_meta.st_size;

	uint8_t *data = vmalloc( file_size );

	if( fread( data, file_size, 1, f ) != 1 ) {
		vdf( "Read failed on %s, fread returned not 1.\n", font->info.path );
		return;
	}
	#endif

	font->type = VUI_FONT_TYPE_TTF;
	font->size = 13;
	font->info.height = 13;
	font->info.width = 13;
	font->glyph_count = 256;
	font->ttf_bitmaps = vmalloc( font->glyph_count * sizeof(font_ttf_bitmap) );
	
	for( int i = 0; i < font->glyph_count; i++ ) {
		//font->ttf_bitmaps[i].pixel = vmalloc( 2 * font->size * font->size ); 
	}

	font->sft.xScale = font->size;
	font->sft.yScale = font->size,
	font->sft.flags = SFT_DOWNWARD_Y;

	//sft.font = sft_loadfile("./fira_code.ttf");
	font->sft.font = sft_loadmem( data, file_size );

	if( font->sft.font == NULL ) {
		vdf( "sft font failed to load" );
	}

	SFT_LMetrics line_metrics;
	sft_lmetrics( &font->sft, &line_metrics );
	//vdf( "asc: %f    dsc: %f    lineGap: %f\n", line_metrics.ascender, line_metrics.descender, line_metrics.lineGap );
	font->info.height = 13;

	SFT_Glyph test_glyph;
	sft_lookup( &font->sft, 'V', &test_glyph );
	SFT_GMetrics glyph_metrics;
	sft_gmetrics( &font->sft, test_glyph, &glyph_metrics );
	font->info.width = glyph_metrics.advanceWidth;

	//vdf( "%f\n", glyph_metrics.advanceWidth );


	SFT_Glyph v;
	

/* 	memset( img.pixels, 0, 2 * 20 * 20 );
	vdf( "lookup result: %d\n", sft_lookup( &sft, 0xf0e1e, &v ) );
	vdf( "render result: %d\n", sft_render( &sft, v, img ) );
	memcpy( font->ttf_bitmaps[0].pixel, img.pixels, 2 * 20 * 20 );

	memset( img.pixels, 0, 2 * 20 * 20 );
	vdf( "lookup result: %d\n", sft_lookup( &sft, 0xf0cdf, &v ) );
	vdf( "render result: %d\n", sft_render( &sft, v, img ) );
	memcpy( font->ttf_bitmaps[1].pixel, img.pixels, 2 * 20 * 20 ); */

	void *pix_store = vmalloc(2 * 100 * 100);

	for( int i = 0; i < 256; i++ ) {
		//vdf( "rendering %d\n", i );
		int lookup_res = sft_lookup( &font->sft, dos_code_page[i], &v );

		//vdf( "lookup_res: %d\n", lookup_res );
		
		SFT_GMetrics glyph_metrics;
		int gmetrics_res = sft_gmetrics( &font->sft, v, &glyph_metrics );
		//vdf( "gmetrics_res: %d\n", gmetrics_res );

		font->ttf_bitmaps[i].x_offset = glyph_metrics.leftSideBearing;
		font->ttf_bitmaps[i].y_offset = -glyph_metrics.yOffset;
		font->ttf_bitmaps[i].advance = glyph_metrics.advanceWidth;
		font->ttf_bitmaps[i].width = glyph_metrics.minWidth;
		font->ttf_bitmaps[i].height = glyph_metrics.minHeight;
		font->ttf_bitmaps[i].pixel = vmalloc( 2 * font->ttf_bitmaps[i].width * font->ttf_bitmaps[i].height ); 
		
		memset( pix_store, 0, 2 * 100 * 100 );

		SFT_Image img = {
			.pixels = pix_store,
			.width = glyph_metrics.minWidth,
			.height = glyph_metrics.minHeight
		};

		//vdf( "\'%c\' y_offset: %d\n", i, font->ttf_bitmaps[i].y_offset );
		//memset( img.pixels, 0, 2 * font->ttf_bitmaps[i].width * font->ttf_bitmaps[i].height );
		int render_res = sft_render( &font->sft, v, img );
		//vdf( "render_res: %d\n", render_res );

		memcpy( font->ttf_bitmaps[i].pixel, img.pixels, 2 * font->ttf_bitmaps[i].width * font->ttf_bitmaps[i].height);
	}
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
	font->aa_mask = vmalloc( sizeof(font_bitmap) * font->info.num_glyphs);

	

	#ifdef KDEBUG_FONT_LOAD_PSF
	vdf( "Data Start: 0x%016llx\n", data );
    #endif

	// Thank you to whomever wrote this table back in 2010: https://stackoverflow.com/questions/2602823/in-c-c-whats-the-simplest-way-to-reverse-the-order-of-bits-in-a-byte
	uint8_t reverse_map[] = {
        0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
        0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
        0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
        0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
        0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
        0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
        0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
        0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
        0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
        0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
        0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
        0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
        0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
        0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
        0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
        0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
        0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
        0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
        0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
        0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
        0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
        0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
        0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
        0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
        0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
        0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
        0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
        0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
        0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
        0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
        0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
        0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
    };

	if( font->info.width <= 8 ) {
		uint8_t *start = (uint8_t *)(data + header_size);

		for( int i = 0; i < font->info.num_glyphs; i++ ) {
			font->bitmaps[i].num = i;

			for( int j = 0; j < font->info.height; j++ ) {
				//font->bitmaps[i].pixel_row[j] = *start << 8;
				font->bitmaps[i].pixel_row[j] = reverse_map[*start];
			
				*start++;
			}
		}
	} else {
		uint16_t *start = (uint16_t *)(data + header_size);
		for( int i = 0; i < font->info.num_glyphs; i++ ) {
			font->bitmaps[i].num = i;

			for( int j = 0; j < font->info.height; j++ ) {
				font->bitmaps[i].pixel_row[j] = reverse_map[*start >> 8] << 8 | reverse_map[*start & 0xFF];
				
				*start++;
			}
		}
	}
	
    return true;
}

#define font_get_pixel(charnum,row,col) ((font->bitmaps[charnum].pixel_row[row] >> col) & 1)
#define font_pixelrow(charnum,row) font->bitmaps[charnum].pixel_row[row]
#define font_aapixelrow(charnum,row) font->aa_mask[charnum].pixel_row[row]
#define font_aapixelrow(charnum,row) font->aa_mask[charnum].pixel_row[row]
#define font_set_aapixel(charnum,row,col) font_aapixelrow(charnum,row) = font_aapixelrow(charnum,row) | (1 << col) 

void vui_font_create_aa_mask( vui_font *font, uint16_t char_num ) {
	if( font == NULL ) {
		return;
	}

	bool patterns[6][4] = {
		{true, false, false, true},
		{true, true, true, false},
		{true, true, false, true},
		{false, true, true, false},
		{false, true, true, true},
		{true, false, true, true}
	};

	// Loop through each pattern
	for( int pattern_num = 0; pattern_num < 6; pattern_num++ ) {
		//vdf( "Pattern: %d\n", pattern_num );

		// Look at each two lines (by the top line), stop when it's the height - 1
		for( uint16_t top_line = 0; top_line < (font->info.height - 1); top_line++ ) {
			//vdf( "Top Line: %d\n", top_line );

			// Look at each column progressively from left to right, col 2 is the last one
			for( uint16_t col = font->info.width - 1; col > 0; col-- ) {
				//vdf( "Col %d  ", col );

				if( font_get_pixel(char_num, top_line, col) == patterns[pattern_num][0] &&
					font_get_pixel(char_num, top_line, col - 1) == patterns[pattern_num][1] &&
					font_get_pixel(char_num, top_line + 1, col) == patterns[pattern_num][2] &&
					font_get_pixel(char_num, top_line + 1, col - 1) == patterns[pattern_num][3]
				) {
					if( font_get_pixel(char_num, top_line, col) == 0 ) { font_set_aapixel(char_num, top_line, col); }
					if( font_get_pixel(char_num, top_line, col - 1) == 0 ) { font_set_aapixel(char_num, top_line, col - 1); }
					if( font_get_pixel(char_num, top_line + 1, col) == 0 ) { font_set_aapixel(char_num, top_line + 1, col); }
					if( font_get_pixel(char_num, top_line + 1, col - 1) == 0 ) { font_set_aapixel(char_num, top_line + 1, col - 1); }
				}
			}

			//vdf( "\n" );
		}
	}

	// Print the pattern
	/* for( uint16_t i = 0; i < font->info.height; i++ ) {
		vdf( "row: %02d \"", i );

		for( int j = font->info.width - 1; j >= 0; j-- ) {			
			if( (font->bitmaps[char_num].pixel_row[i] >> j & 1) == 1 ) {
				vdf( "*" );
			} else if( (font->aa_mask[char_num].pixel_row[i] >> j & 1) == 1  ) {
				vdf( "." );
			} else {
				vdf( " " );
			}	

			if( ((font->bitmaps[char_num].pixel_row[i] >> j & 1) == 1 ) && ((font->aa_mask[char_num].pixel_row[i] >> j & 1) == 1 ) ) { vdf( "COLLISON! i = %d, j = %d\n", i, j ); }	
		}
		vdf( "\"\n" );
	} */

// 2. Go through the bitmap, looking for each pattern in 2x2 chunks

/*
* - Pixel
_ - Empty
. - AA point
 
Patterns:                           Pixel Numbers:

*_  **  *_  *_  **  **  **          12
__  __  *_  _*  *_  _*  **          34

_*  _*  _*   _*
__  *_  _*   **

__  __  __
*_  _*  **

__
_*


AA Applied:                         AA Only Patterns w/Pix number:

*_  **  *_  *.  **  **  **  *.       *.  **  **    true, false, false, true    true, true, true, false    true, true, false, true
__  __  *_  .*  *.  .*  **  **       .*  *.  .*

_*  .*  _*   .*                      .*  .*  *.      false, true, true, false    false, true, true, true    true, false, true, true
__  *.  _*   **                      *.  **  **

__  __  __
*_  _*  **

__
_*

pix1_t/f = true
pix2_t/f = false
pix3_t/f = false
pix4_t/f = true

for( int top_line = 0; top_line < font.width; top_line++ ) {
	for( int col = font.width; col < font_width; col-- ) {
		if( bitmap[top_line][col] == PIX1_T/F && bitmap[top_line][col - 1] == PIX2_T/F ) {
			if( bitmap[top_line+1][col] == PIX3_T/F && bitmap[top_line][col - 1] == PIX4_T/F ) {
				if pix1 == empty then fill with aa color
				if pix2 == empty then fill with aa color
				if pix3 == empty then fill with aa color
				if pix4 == empty then fill with aa color
			}
		}
	}
}

*/



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