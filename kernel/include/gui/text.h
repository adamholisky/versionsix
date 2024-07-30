#ifndef VIOS_GUI_TEXT_INCLUDED
#define VIOS_GUI_TEXT_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

void text_test( void );
void text_put_string( char *str );

typedef struct {
	uint16_t pixel_top;
	uint16_t pixel_left;
	uint16_t pixel_width;
	uint16_t pixel_height;
} vui_text;

typedef struct {
	uint16_t num;
	uint16_t pixel_row[20];
} font_bitmap;

#define PSF1_FONT_MAGIC 0x0436

typedef struct {
    uint16_t magic; // Magic bytes for identification.
    uint8_t fontMode; // PSF font mode.
    uint8_t characterSize; // PSF character size.
} psf1_header;

#define PSF_FONT_MAGIC 0x864ab572

typedef struct {
    uint32_t magic;         /* magic bytes to identify PSF */
    uint32_t version;       /* zero */
    uint32_t headersize;    /* offset of bitmaps in file, 32 */
    uint32_t flags;         /* 0 if there's no unicode table */
    uint32_t numglyph;      /* number of glyphs */
    uint32_t bytesperglyph; /* size of each glyph */
    uint32_t height;        /* height in pixels */
    uint32_t width;         /* width in pixels */
} psf_font;

void vui_text_initalize( vui_text *txt, uint16_t top, uint16_t left, uint16_t width, uint16_t height );
void vui_text_put_char( vui_text *txt, uint8_t c );
void vui_text_put_char_unicode( vui_text *txt, uint32_t c );
void vui_text_put_char_at( vui_text *txt, uint8_t c, uint16_t x, uint16_t y );
void vui_text_put_char_at_unicode( vui_text *txt, uint32_t c, uint16_t x, uint16_t y );
void vui_text_put_char_at_with_color( vui_text *txt, uint8_t c, uint16_t x, uint16_t y, uint32_t foreground_color, uint32_t background_color );
void vui_text_put_char_at_with_color_unicode( vui_text *txt, uint32_t c, uint16_t x, uint16_t y, uint32_t foreground_color, uint32_t background_color );
void vui_text_put_string( vui_text *txt, char *str );

void load_font_psf( void );
void load_font_bdf( void );


#ifdef __cplusplus
}
#endif
#endif