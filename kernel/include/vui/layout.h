#ifndef VIOS_VUI_LAYOUT_INCLUDED
#define VIOS_VUI_LAYOUT_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include "vui/vui.h"

#define VUI_LAYOUT_GRID_3x3 6

#define VUI_LAYOUT_PADDING 10

#define VUI_LAYOUT_CENTER 0
#define VUI_LAYOUT_LEFT 1
#define VUI_LAYOUT_RIGhT 2
#define VUI_LAYOUT_TOP 3
#define VUI_LAYOUT_BOTTOM 4

typedef struct {
	vui_handle H;
	uint8_t span;
	
	uint16_t absolute_x;
	uint16_t absolute_y;
	uint16_t width;
	uint16_t height;

	uint8_t vert_position;
	uint8_t horz_position;

	vui_handle parent;
} vui_layout_cell;

typedef struct {
	vui_layout_cell *col;
	
	uint16_t absolute_x;
	uint16_t absolute_y;
	uint16_t width;
	uint16_t height;
} vui_layout_row;

typedef struct {
	vui_layout_row *row;

	uint16_t rows;
	uint16_t cols;

	uint16_t padding;
	uint16_t absolute_x;
	uint16_t absolute_y;
	uint16_t width;
	uint16_t height;

	vui_handle parent;
} vui_layout;

vui_layout *vui_layout_init( uint8_t type, vui_handle parent );
void vui_layout_dump_diagnostic( vui_layout *layout );
vui_layout_cell *vui_layout_get_next_free( vui_layout *layout );

void *vui_layout_add_button( vui_layout_cell *cell, char *name, char *text );


#ifdef __cplusplus
}
#endif
#endif