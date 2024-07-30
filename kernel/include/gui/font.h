#ifndef VIOS_GUI_FONT_INCLUDED
#define VIOS_GUI_FONT_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t height;
    uint16_t width;
    uint16_t num_glyphs;
} font_info;

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

font_bitmap *font_get_main_bitmap( void );
font_info *font_get_main_info( void );
bool font_load_psf( char *font_path );
bool font_load_bdf( char *font_path );

#ifdef __cplusplus
}
#endif
#endif