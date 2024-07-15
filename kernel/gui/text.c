#include <kernel_common.h>
#include <framebuffer.h>
#include <gui/gui.h>
#include <gui/text.h>

#define USE_DOS_CODEPAGE_437_TRANSLATION
#ifdef USE_DOS_CODEPAGE_437_TRANSLATION
static uint16_t codepage_conversion[256] = {
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
#endif

extern framebuffer_state fb_state;
extern char *u_vga16;
extern char *unifont_vvi;

#define SSFN_IMPLEMENTATION
#define SSFN_realloc krealloc
#define SSFN_free    kfree
#include <gui/ssfn.h>
ssfn_t ssfn_context;
ssfn_buf_t ssfn_buffer;

void vui_text_initalize( vui_text *txt, uint16_t top, uint16_t left, uint16_t width, uint16_t height ) {
	txt->pixel_top = top;
	txt->pixel_left = left;
	txt->pixel_width = width;
	txt->pixel_height = height;

	//ssfn_src = (ssfn_font_t *)&unifont_vvi;
	//ssfn_src = (ssfn_font_t *)&u_vga16;

	memset( &ssfn_context, 0, sizeof(ssfn_context) );
	ssfn_load( &ssfn_context, &u_vga16 );
	ssfn_buffer.ptr = fb_state.fb_info->address;
	ssfn_buffer.w = width;
	ssfn_buffer.h = height;
	ssfn_buffer.p = fb_state.fb_info->pitch;
	ssfn_buffer.x = left;
	ssfn_buffer.y = top;
	ssfn_buffer.fg = 0x00000000;

	int err = ssfn_select( &ssfn_context, SSFN_FAMILY_MONOSPACE, NULL, SSFN_STYLE_REGULAR, 16 );

	if( err < 0 ) {
		debugf( "ssfn_select err: %d\n", err );
		return;
	}


	/* ssfn_dst.ptr = (uint8_t *)fb_state.fb_info->address;
	ssfn_dst.w = width;
	ssfn_dst.h = height;
	ssfn_dst.p = fb_state.fb_info->pitch;
	ssfn_dst.x = left;
	ssfn_dst.y = top;
	ssfn_dst.fg = 0x00FF0000; */
}

void vui_text_put_char( vui_text *txt, uint8_t c ) {
	uint8_t temp_string[2] = {c, 0};
	ssfn_render( &ssfn_context, &ssfn_buffer, temp_string );
}

void vui_text_put_char_unicode( vui_text *txt, uint32_t c ) {
	uint8_t temp_string[2] = {c, 0};
	ssfn_render( &ssfn_context, &ssfn_buffer, temp_string );
}

void vui_text_put_char_at( vui_text *txt, uint8_t c, uint16_t x, uint16_t y ) {
	vui_text_put_char_at_unicode( txt, c, x, y );
}

void vui_text_put_char_at_unicode( vui_text *txt, uint32_t c, uint16_t x, uint16_t y )  {
	uint8_t temp_string[2] = {c, 0};

	ssfn_buffer.x = x;
	ssfn_buffer.y = y;
	ssfn_render( &ssfn_context, &ssfn_buffer, temp_string );
}

void vui_text_put_char_at_with_color( vui_text *txt, uint8_t c, uint16_t x, uint16_t y, uint32_t foreground_color, uint32_t background_color ) {
	vui_text_put_char_at_with_color_unicode( txt, (uint32_t)c, x, y, foreground_color, background_color );
}

void vui_text_put_char_at_with_color_unicode( vui_text *txt, uint32_t c, uint16_t x, uint16_t y, uint32_t foreground_color, uint32_t background_color ) {
	uint8_t final_chars[4] = {(uint8_t)c, 0, 0, 0};
	int err = 0;

	//debugf( "c = 0x%X '%c'\n", c, c );

	#ifdef USE_DOS_CODEPAGE_437_TRANSLATION
	if( c < 255 ) {
		utf8_encode( final_chars, codepage_conversion[c] );
	} else {
		//utf8_encode( final_chars, c );
	}
	#endif

	// Clear the background, ssfn doesn't
	fb_primative_fill_rect( ssfn_buffer.ptr, background_color, x, y + 10, x + 8, y + 14 + 18 );

	ssfn_buffer.fg = foreground_color | FULL_ALPHA;
	ssfn_buffer.bg = 0;
	ssfn_buffer.x = x;
	ssfn_buffer.y = y + 14;
	err = ssfn_render( &ssfn_context, &ssfn_buffer, &final_chars );

	if( err < 0 ) {
		debugf( "ssfn_render[%d] err: %d\n", 0, err );
		return;
	}
}

void vui_text_put_string( vui_text *txt, char *str ) {
	ssfn_render( &ssfn_context, &ssfn_buffer, str );
}

/**
 * Encode a code point using UTF-8
 * 
 * @author Ondřej Hruška <ondra@ondrovo.com>
 * @license MIT
 * 
 * @param out - output buffer (min 5 characters), will be 0-terminated
 * @param utf - code point 0-0x10FFFF
 * @return number of bytes on success, 0 on failure (also produces U+FFFD, which uses 3 bytes)
 */
int utf8_encode(uint8_t *out, uint32_t utf)
{
  if (utf <= 0x7F) {
    // Plain ASCII
    out[0] = (char) utf;
    out[1] = 0;
    return 1;
  }
  else if (utf <= 0x07FF) {
    // 2-byte unicode
    out[0] = (char) (((utf >> 6) & 0x1F) | 0xC0);
    out[1] = (char) (((utf >> 0) & 0x3F) | 0x80);
    out[2] = 0;
    return 2;
  }
  else if (utf <= 0xFFFF) {
    // 3-byte unicode
    out[0] = (char) (((utf >> 12) & 0x0F) | 0xE0);
    out[1] = (char) (((utf >>  6) & 0x3F) | 0x80);
    out[2] = (char) (((utf >>  0) & 0x3F) | 0x80);
    out[3] = 0;
    return 3;
  }
  else if (utf <= 0x10FFFF) {
    // 4-byte unicode
    out[0] = (char) (((utf >> 18) & 0x07) | 0xF0);
    out[1] = (char) (((utf >> 12) & 0x3F) | 0x80);
    out[2] = (char) (((utf >>  6) & 0x3F) | 0x80);
    out[3] = (char) (((utf >>  0) & 0x3F) | 0x80);
    out[4] = 0;
    return 4;
  }
  else { 
    // error - use replacement character
    out[0] = (char) 0xEF;  
    out[1] = (char) 0xBF;
    out[2] = (char) 0xBD;
    out[3] = 0;
    return 0;
  }
}