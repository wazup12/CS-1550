#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include <asm/termbits.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>

#include "graphics.h"

// DELETE THIS
#include <stdio.h>

// program variables
int SCREEN_WIDTH;
int SCREEN_HEIGHT;
int BITS_PER_PIXEL;
int BUFFER_SIZE;
int FRAME_BUFFER_FD;
char *FRAME_MAP;
struct termios term_settings;
fd_set read_fds;

// open, ioctl, mmap
void init_graphics() {
  printf("yo");
  int FRAME_BUFFER_FD = open("/dev/fb0", O_RDWR);

  struct fb_var_screeninfo vinfo;
  struct fb_fix_screeninfo finfo;

  ioctl(FRAME_BUFFER_FD, FBIOGET_VSCREENINFO, &vinfo);
  ioctl(FRAME_BUFFER_FD, FBIOGET_FSCREENINFO, &finfo);

  SCREEN_WIDTH = vinfo.xres;
  SCREEN_HEIGHT = vinfo.yres;
  printf("%u x %u\n", SCREEN_WIDTH, SCREEN_HEIGHT);
  // BITS_PER_PIXEL = 8 * finfo.line_length / SCREEN_WIDTH;
  BUFFER_SIZE = finfo.line_length * SCREEN_HEIGHT;

  printf("yoyo\n");
  FRAME_MAP = mmap(
    NULL,
    BUFFER_SIZE,
    PROT_READ | PROT_WRITE,
    MAP_SHARED,
    FRAME_BUFFER_FD,
    0
  );

  printf("yoyoyo\n");
  FD_ZERO(&read_fds);
  printf("yoyoyoyo\n");
  FD_SET(STDIN_FILENO, &read_fds);
  printf("yoyoyoyoyo\n");

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
}

void exit_graphics() {
  // close files
  close(FRAME_BUFFER_FD);
  // unmap memory
  munmap(FRAME_MAP, BUFFER_SIZE);
  // reset terminal settings
  term_settings.c_lflag |= ICANON;
  term_settings.c_lflag |= ECHO;
  ioctl(0, TCSETS, &term_settings);

  // reset read_fds
  FD_ZERO(&read_fds);
}

char getkey() {
  // read from stdin if there is input

  if(select(1, &read_fds, NULL, NULL, 0) > 0) {
    char c;
    read(STDIN_FILENO, &c, 1);
    return c;
  }
}

void sleep_ms(long ms) {
  // if over 999999999 nanoseconds, convert rest seconds
  time_t seconds = 0;
  long nanoseconds = 0;
  if (ms > 999) {
    seconds = (ms-999)/1000;
    nanoseconds = 999 * 1000000;
  } else {
    nanoseconds = ms * 1000000;
  }

  struct timespec ts;
  ts.tv_sec = seconds;
  ts.tv_nsec = nanoseconds;

  nanosleep(&ts, NULL);
}

// TODO implement
void clear_screen() {
  return;
}

// TODO implement
void draw_pixel(int x, int y, color_t color) {
  return;
}

// TODO implement
void draw_rect(int x, int y, int width, int height, color_t color) {

  return;
}

// TODO implement
void draw_text(int x, int y, const char *text, color_t color) {

  return;
}

// TODO implement
void draw_circle(int x, int y, int r, color_t c) {

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