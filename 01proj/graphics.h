#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <stdint.h>

/*
f   ba    54   0
<-r-><-g--><-b->
*/
typedef uint16_t color_t;

// ----- util functions ----- //
void init_graphics();			//open, ioctl, mmap
void exit_graphics();			//ioctl
char getkey();						//select, read
void sleep_ms(long ms);		//nanosleep
void clear_screen();			//write

color_t rgb_to_color_t(int r, int g, int b);

// ----- drawing functions ----- //
void draw_pixel(int x, int y, color_t color);
void draw_rect(int x, int y, int width, int height, color_t color);
void draw_text(int x, int y, const char *text, color_t color);
void draw_circle(int x, int y, int r, color_t c);
#endif
