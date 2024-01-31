#include "graphics.h"
#include <stdio.h>

color_t WHITE, RED, GREEN, BLUE;
const char* msg = "Hello, world!\0";
const char* qwerty = "QwErTyUiOpAsDfGhJkL\0";
const char* prompt = "Type a char: \0";

void init_colors() {
	WHITE = hex_to_color_t(255, 255, 255);
	RED = hex_to_color_t(255, 0, 0);
	GREEN = hex_to_color_t(0, 255, 0);
	BLUE = hex_to_color_t(0, 0, 255);
}

int main(void) {
	init_colors();

	init_graphics();
	clear_screen();
	sleep_ms(1000);

	draw_pixel(0 ,0, WHITE);
	draw_pixel(1 ,0, RED);
	draw_pixel(2 ,0, GREEN);
	draw_pixel(2 ,2, BLUE);
	sleep_ms(500);

	draw_rect_filled(50, 50, 50, 30, GREEN);
	draw_rect(100, 50, 50, 30, GREEN);

	draw_circle_filled(100, 100, 30, RED);
	draw_circle(200, 200, 70, BLUE);

	draw_letter(400, 220, 'A', WHITE);
	draw_text(100, 300, msg, RED);
	sleep_ms(100);
	draw_text(100, 316, msg, GREEN);
	sleep_ms(100);
	draw_text(100, 332, msg, BLUE);

	sleep_ms(300);

	draw_text_typed(300, 300, qwerty, RED, 70);
	draw_text_typed(300, 316, qwerty, GREEN, 90);
	draw_text_typed(300, 332, qwerty, BLUE, 110);

	draw_text_typed(100, 400, prompt, WHITE, 60);

	// must be int otherwise does not evaluate correctly
	int input = -1;
	while(input == -1) {
		input = getkey();
	}

	draw_letter(100+13*8, 400, input, RED);

	exit_graphics();
}
