#ifndef VIOS_VUI_INCLUDED
#define VIOS_VUI_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

#include "vit.h"
#include "vui/font.h"

#define VUI_HANDLES_MAX 512

#define COLOR_RGB_BLACK 0x00000000
#define COLOR_RGB_WHITE 0x00FFFFFF
#define COLOR_RGB_BLUE  0x000000FF
#define COLOR_RGB_GREEN 0x0000FF00
#define COLOR_RGB_RED   0x00FF0000

#define VUI_HANDLE_TYPE_WINDOW 1
#define VUI_HANDLE_TYPE_LABEL 2
#define VUI_HANDLE_TYPE_BUTTON 3
#define VUI_HANDLE_TYPE_DESKTOP 4
#define VUI_HANDLE_TYPE_CONSOLE 5
#define VUI_HANDLE_TYPE_MENUBAR 6
#define VUI_HANDLE_TYPE_MENU 7
#define VUI_HANDLE_TYPE_MENUITEM 8

#define VUI_PRIORITY_FRONT 0x1
#define VUI_PRIORITY_MENU 0x10
#define VUI_PRIORITY_WINDOW 0x20
#define VUI_PRIORITY_DESKTOP 0xFFFFFFFF

#define VUI_DRAW_FLAGS_NONE 				0
#define VUI_DRAW_FLAGS_DISABLE_SMOOTHING	(1 << 0)
#define VUI_DRAW_FLAGS_IMMEDIATE 			(1 << 1)
#define VUI_DRAW_FLAGS_TRANSPARENT			(1 << 2)

typedef uint32_t vui_handle;

typedef struct {
	vui_handle H;
	uint16_t type;
	void *data;
} vui_handles;

typedef struct {
	vui_handle H;

	void *next;
} vui_handle_list;

typedef struct {
	vui_handle H;
	uint8_t type;
	uint32_t flags;
	char name[50];

	// Keyboard
	char key;

	// Mouse
	uint16_t x;
	uint16_t y;
	uint16_t x_displacement;
	uint16_t y_displacement;
} vui_event;

typedef struct {
	uint32_t desktop;
	uint32_t border;
	uint32_t window_background;
	uint32_t window_title_bar_background;
	uint32_t window_title_bar_foreground;
	uint32_t button_foreground;
	uint32_t button_background;
	uint32_t button_active;
	uint32_t button_hover;
	uint32_t menubar_background;
	uint32_t menubar_foreground;
} vui_theme;

typedef struct {
	uint32_t *fb;
	uint32_t *buffer;
	uint16_t width;
	uint16_t height;
	uint16_t pitch;

	vui_theme active_theme;

	vui_handles handles[VUI_HANDLES_MAX];

	bool immediate_output;
	bool disable_font_smoothing;

	vui_handle_list dispatchers;
	vui_handle handle_next;

	vui_handle last_hover;
} vui_core;

typedef struct {
	void (*event_handler)(vui_event *);

	void (*default_on_mouse_down)(vui_event *);
	void (*default_on_mouse_enter)(vui_event *);
	void (*default_on_mouse_exit)(vui_event *);
	void (*default_on_mouse_move)(vui_event *);
	void (*default_on_mouse_up)(vui_event *);

	void (*on_mouse_down)(vui_event *);
	void (*on_mouse_enter)(vui_event *);
	void (*on_mouse_exit)(vui_event *);
	void (*on_mouse_up)(vui_event *);

	void (*draw_from_struct)(void *);
	void (*draw_from_handle)(vui_handle);
} vui_operations;

typedef struct {
	uint16_t type;
	vui_handle handle;
	char name[50];
	vui_handle parent;
	vui_handle_list children;
	uint32_t priority;
	vui_operations ops;

	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	uint16_t absolute_x;
	uint16_t absolute_y;

	uint16_t inner_x;
	uint16_t inner_y;
	uint16_t inner_width;
	uint16_t inner_height;

	bool is_hover;
} vui_common;

#define INSERT_VUI_COMMON uint16_t type; \
vui_handle handle; \
char name[50]; \
vui_handle parent; \
vui_handle_list children; \
uint32_t priority; \
vui_operations ops; \
\
uint16_t x; \
uint16_t y; \
uint16_t width; \
uint16_t height; \
uint16_t absolute_x; \
uint16_t absolute_y; \
\
uint16_t inner_x; \
uint16_t inner_y; \
uint16_t inner_width; \
uint16_t inner_height; \
\
bool is_hover; 


/**************************************/
/* Core Functions                     */
/**************************************/
void vui_init( uint32_t *fb_addr, uint16_t width, uint16_t height );
void vui_refresh( void );
void vui_refresh_rect( uint16_t x, uint16_t y, uint16_t width, uint16_t height );
void vui_refresh_handle( vui_handle H );

/**************************************/
/* Handle Management Functions                     */
/**************************************/
vui_handle vui_allocate_handle( uint16_t type );
void vui_create_cleanup( vui_handle H );
void vui_add_to_parent( vui_handle parent, vui_handle child );
uint16_t vui_get_type_from_master_list( vui_handle H );
void vui_handle_set_name( vui_handle H, char *name );
char *vui_handle_get_name( vui_handle H);
void vui_set_handle_data( vui_handle H, void *data );
void *vui_get_handle_data( vui_handle H);

bool vui_handle_list_add( vui_handle_list *list, vui_handle handle_to_add );
void vui_sort_list_by_priority( vui_handle_list *list );

/**************************************/
/* Drawing                 */
/**************************************/
vui_theme *vui_get_active_theme( void );
void vui_draw( vui_handle H );
void vui_draw_handle( vui_handle H );
void vui_draw_rect( uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color );
void vui_draw_string( char *s, uint16_t x, uint16_t y, uint32_t fg, uint32_t bg, vui_font *font, uint64_t flags );
void vui_draw_char( uint16_t char_num, uint16_t x, uint16_t y, uint32_t fg, uint32_t bg, vui_font *font, uint64_t flags );
void vui_move_rect( uint32_t dest_x, uint32_t dest_y, uint32_t dest_w, uint32_t dest_h, uint32_t src_x, uint32_t src_y, uint32_t src_w, uint32_t src_h );

#ifdef __cplusplus
}
#endif
#endif