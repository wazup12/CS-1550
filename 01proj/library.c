#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include <asm/termbits.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>

#include "graphics.h"
#include "iso_font.h"

// program variables
int SCREEN_WIDTH;
int SCREEN_HEIGHT;
int BYTES_PER_PIXEL;
int BITS_PER_PIXEL;
int BUFFER_SIZE;
int FRAME_BUFFER_FD;
char *FRAME_MAP;
struct termios term_settings;
fd_set read_fds;
char* CLEAR = "\033[2J";

// open, ioctl, mmap
void init_graphics() {
  int FRAME_BUFFER_FD = open("/dev/fb0", O_RDWR);

  struct fb_var_screeninfo vinfo;
  struct fb_fix_screeninfo finfo;

  ioctl(FRAME_BUFFER_FD, FBIOGET_VSCREENINFO, &vinfo);
  ioctl(FRAME_BUFFER_FD, FBIOGET_FSCREENINFO, &finfo);

  SCREEN_WIDTH = vinfo.xres;
  SCREEN_HEIGHT = vinfo.yres;
  BYTES_PER_PIXEL = finfo.line_length / SCREEN_WIDTH;
  BITS_PER_PIXEL = 8 * BYTES_PER_PIXEL;
  BUFFER_SIZE = finfo.line_length * SCREEN_HEIGHT;

  FRAME_MAP = mmap(
    NULL,
    BUFFER_SIZE,
    PROT_READ | PROT_WRITE,
    MAP_SHARED,
    FRAME_BUFFER_FD,
    0
  );

  FD_ZERO(&read_fds);
  FD_SET(STDIN_FILENO, &read_fds);

  // explanation of termios settings
  // ICANNON  = 0b0010
  // ~ICANNON = 0b1101
  // &= with the negation forces the target bit
  //     to be 0 in term_settings
  // |= with the original forces the target bit
  //     to be 1 in term_settings
  ioctl(0, TCGETS, &term_settings);
  term_settings.c_lflag &= ~ICANON;
  term_settings.c_lflag &= ~ECHO;
  ioctl(0, TCSETS, &term_settings);

  return;
}

void exit_graphics() {
  // reset term settings
  ioctl(0, TCGETS, &term_settings);
  term_settings.c_lflag |= (ICANON+ECHO);
  ioctl(0, TCSETS, &term_settings);

  // reset read_fds (redundant?)
  FD_ZERO(&read_fds);

  // unmap frame buffer from memory
  munmap(FRAME_MAP, BUFFER_SIZE);

  // close files
  close(FRAME_BUFFER_FD);
  return;
}

char getkey() {
  // read from stdin if there is input
  int character = -1;
  if(select(1, &read_fds, NULL, NULL, 0) > 0) {
    read(STDIN_FILENO, &character, 1);
  }
  return character;
}

void sleep_ms(long ms) {
  // if over 999999999 nanoseconds, convert rest seconds
  time_t seconds = 0;
  long nanoseconds;
  if (ms > 999) {
    seconds = ms/1000;
    nanoseconds = (ms%1000) * 1000000;
  } else {
    nanoseconds = ms * 1000000;
  }

  struct timespec ts;
  ts.tv_sec = seconds;
  ts.tv_nsec = nanoseconds;

  int test_return;
  test_return = nanosleep(&ts, NULL);
  return;
}

void clear_screen() {
  write(STDOUT_FILENO, CLEAR, 4);
  return;
}

void draw_pixel(int x, int y, color_t color) {
  char* address = FRAME_MAP
      + SCREEN_WIDTH * BYTES_PER_PIXEL * y 
      + BYTES_PER_PIXEL * x;

  // first byte
  *address = color;
  // second byte
  *(address+1) = color>>8;
  return;
}

void draw_rect(int x, int y, int width, int height, color_t color) {
  // per requirements, bounds are [x1, x1+w]
  // therefore must be <= width
  int i;
  for(i = 0; i <= width; ++i) {
    draw_pixel(x+i, y, color);
    draw_pixel(x+i, y+height, color);
  }

  for(i = 0; i <= height; ++i) {
    draw_pixel(x, y+i, color);
    draw_pixel(x+width, y+i, color);
  }
  return;
}

void draw_rect_filled(int x, int y, int width, int height, color_t color) {
  int i, j;
  for(i = 0; i <= width; ++i)
    for(j = 0; j <= height; ++j)
      draw_pixel(x+i, y+j, color);
  return;
}

void draw_letter(int x, int y, char c, color_t color) {
  int iso_index = c*16;
  int row, i, curr;
  for(row = 0; row < 16; ++row) {
    curr = iso_font[iso_index+row];
    for(i = 0; i < 8; ++i) {
      if((curr>>i) % 2 == 1) {
        draw_pixel(x+i, y+row, color);
      }
    }
  }

  return;
}

void draw_text(int x, int y, const char *text, color_t color) {
  const char *curr = text; 
  int i = 0;
  while(*curr != '\0') {
    draw_letter(x+i, y, *curr, color);
    curr++;
    i+=8;
  }

  return;
}

void draw_text_typed(int x, int y, const char *text, color_t color, long delay) {
  const char *curr = text; 
  int i = 0;
  while(*curr != '\0') {
    draw_letter(x+i, y, *curr, color);
    curr++;
    i+=8;
    sleep_ms(delay);
  }
  return;
}

// modification of draw_circle_filled
// only draws if the absolute value of the
// difference is within a value
void draw_circle(int x, int y, int r, color_t c) {
  int tempX = 0;
  int tempY = 0;
  for(tempY = -r; tempY <= r; ++tempY){
    for(tempX = -r; tempX <= r; ++tempX) {
      int out_value = r*r-(tempX*tempX + tempY*tempY);
      if (out_value < r && out_value > -r)
        draw_pixel(x+tempX, y+tempY, c);
    }
  }
  return;
}

// filled circle is just derived from
// x^2+y^2 = r^2
// (since it is filled in, anything less than r^2 is also filled)
// so it becomes x^2 + y^2 <= r^2
void draw_circle_filled(int x, int y, int r, color_t c) {
  int tempX = 0;
  int tempY = 0;
  for(tempY = -r; tempY <= r; ++tempY){
    for(tempX = -r; tempX <= r; ++tempX) {
      if(tempX*tempX+tempY*tempY <= r*r)
        draw_pixel(x+tempX, y+tempY, c);
    }
  }
  return;
}

// preconditions 
// r is in [0, 31] (5 bits)
// g is in [0, 63] (6 bits)
// b is in [0, 31] (5 bits)
color_t rgb_to_color_t(int r, int g, int b) {
  color_t color = 0;

  color |= r;
  color <<= 6;
  color |= g;
  color <<= 5;
  color |= b;

  return color;
}

// r, g, b in [0,255]
color_t hex_to_color_t(int r, int g, int b) {
  return rgb_to_color_t(r/255*31, g/255*63, b/255*31);
}