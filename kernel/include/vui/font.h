#ifndef VIOS_VUI_FONT_INCLUDED
#define VIOS_VUI_FONT_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include "vui/vui.h"

#define VUI_FONT_TYPE_PSF 1
#define VUI_FONT_TYPE_BDF 2
#define VUI_FONT_TYPE_VDF 3

typedef struct {
    uint16_t height;
    uint16_t width;
    uint16_t num_glyphs;
    uint16_t id;
    char name[50];
    char path[255];
} font_info;

typedef struct {
	uint16_t num;
    
	uint16_t pixel_row[20];
} font_bitmap;

typedef struct {
    font_bitmap *bitmaps;
    font_bitmap *aa_mask;
    font_info info;

    void *next;
} vui_font;

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

void vui_font_initalize( void );
void vui_font_load( uint8_t type, char *name, char *font_path );
vui_font *vui_font_get_font( char *name );
vui_font *vui_font_get_main_font( void );
bool vui_font_load_psf( vui_font *font );
void vui_font_get_bounding_box( char *text, vui_font *font, uint16_t *width, uint16_t *height );
void vui_font_create_aa_mask( vui_font *font, uint16_t char_num );

//bool font_load_bdf( char *font_path );

#ifdef __cplusplus
}
#endif
#endif