#include "vui/vui.h"
#include "vui/layout.h"
#include "vui/button.h"

/*

|     |     |     |     |

*/

extern vui_core vui;

vui_layout *vui_layout_init( uint8_t type, vui_handle parent ) {
	uint8_t rows;
	uint8_t cols;

	switch( type ) {
		case VUI_LAYOUT_GRID_3x3:
		default:
			rows = 3;
			cols = 3;
	}

	vui_common *parent_st = vui_get_handle_data(parent);

	vui_layout *layout = vmalloc(sizeof(vui_layout));
	memset( layout, 0, sizeof(vui_layout) );

	layout->absolute_x = parent_st->inner_x;
	layout->absolute_y = parent_st->inner_y;
	
	//vdf( "Abs X/Y/W/H: x: %d   y: %d    w: %d   h: %d\n", parent_st->inner_x, parent_st->inner_x, parent_st->inner_width, parent_st->inner_height );

	layout->width = parent_st->inner_width;
	layout->height = parent_st->inner_height;

	layout->rows = rows;
	layout->cols = cols;
	layout->parent = parent;
	layout->padding = VUI_LAYOUT_PADDING;

	uint16_t cell_width = ( layout->width - (VUI_LAYOUT_PADDING * (cols+1)) ) / cols;
	uint16_t cell_height = ( layout->height - (VUI_LAYOUT_PADDING * (rows+1)) ) / rows;

	//vdf( "cell    width: %d    height: %d\n", cell_width, cell_height );

	layout->row = vmalloc(sizeof(vui_layout_row) * rows);

	uint16_t cell_y = layout->absolute_y + VUI_LAYOUT_PADDING;

	for( int i = 0; i < rows; i++ ) {
		layout->row[i].col = vmalloc(sizeof(vui_layout_cell) * cols);
		memset( layout->row[i].col, 0, sizeof(vui_layout_cell) * cols );
		
		uint16_t cell_x = layout->absolute_x + VUI_LAYOUT_PADDING;

		for( int j = 0; j < cols; j++ ) {
			layout->row[i].col[j].absolute_x = cell_x;
			layout->row[i].col[j].absolute_y = cell_y;
			layout->row[i].col[j].width = cell_width;
			layout->row[i].col[j].height = cell_height;
			layout->row[i].col[j].parent = parent;

			cell_x = cell_x + VUI_LAYOUT_PADDING + cell_width;
		}

		cell_y = cell_y + VUI_LAYOUT_PADDING + cell_height;
	}

	return layout;
}

void vui_layout_dump_diagnostic( vui_layout *layout ) {
	vdf( "Layout Core Data:\n" );
	vdf( "    Parent: %d\n", layout->parent );
	vdf( "    Rows: %d\n", layout->rows );
	vdf( "    Cols: %d\n", layout->cols );
	vdf( "    Padding: %d\n", layout->padding );

	for(int i = 0; i < layout->rows; i++ ) {
		vdf( "---------------------------------------\n" );

		for(int j = 0; j < layout->cols; j++ ) {
			vdf( "layout[%d][%d]:\n", i, j );
			vdf( "    X: %d\n", layout->row[i].col[j].absolute_x );
			vdf( "    Y: %d\n", layout->row[i].col[j].absolute_y );
			vdf( "    width:  %d\n", layout->row[i].col[j].width );
			vdf( "    height: %d\n", layout->row[i].col[j].height );

		}
	}
}

vui_layout_cell *vui_layout_get_next_free( vui_layout *layout ) {
	for(int i = 0; i < layout->rows; i++ ) {
		for(int j = 0; j < layout->cols; j++ ) {
			if( layout->row[i].col[j].H == 0 ) {
				return &layout->row[i].col[j];
			}
		}
	}

	return NULL;
}

void *vui_layout_add_button( vui_layout_cell *cell, char *name, char *text ) {
	if( cell->H != 0 ) {
		vdf( "Cell already occupied. Aborting.\n" );
		return NULL;
	}

	uint16_t x_pos = cell->absolute_x;
	uint16_t y_pos = cell->absolute_y;

	switch( cell->vert_position ) {
		case VUI_LAYOUT_TOP:
			break;
		case VUI_LAYOUT_BOTTOM:
			y_pos = y_pos + (cell->height) - 35;
			break;
		case VUI_LAYOUT_CENTER:
		default:
			y_pos = y_pos + (cell->height/2) - (35/2);
	}

	cell->H = vui_button_create( text, x_pos, y_pos, cell->width, 35, cell->parent );
	vui_handle_set_name( cell->H, name );

	return vui_get_handle_data( cell->H );
}