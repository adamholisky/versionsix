#ifndef VIOS_GUI_TEXT_INCLUDED
#define VIOS_GUI_TEXT_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

void text_test( void );
void text_put_string( char *str );

class Text {
	private:
		uint16_t pixel_top;
		uint16_t pixel_left;
		uint16_t pixel_width;
		uint16_t pixel_height;
	public:
		void put_char( char c );
		void put_char( char c, uint16_t x, uint16_t y );
		void put_char( char c, uint16_t x, uint16_t y, uint32_t foreground_color, uint32_t background_color );
		void put_string( char *str );

		Text( uint16_t top, uint16_t left, uint16_t width, uint16_t height );
		
};

#ifdef __cplusplus
}
#endif
#endif