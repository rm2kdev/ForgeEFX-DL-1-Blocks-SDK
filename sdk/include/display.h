#ifndef FREEFX_DISPLAY_H
#define FREEFX_DISPLAY_H

#include "ScreenConfig.h"

extern unsigned int display_framebuffer[DISPLAY_BYTES];
/* Message-thread render target. Storage is row-major, MSB first, one byte per
   unsigned int. Targets must fit the physical LCD; NULL restores the LCD.
   Switching targets resets the drawing origin. No allocation or ownership transfer. */
typedef struct DisplaySurface {
    unsigned int *pixels;
    int width, height, stride;
} DisplaySurface;
DisplaySurface display_surface(DisplaySurface *surface);
void display_clear(void);
/* Translate authored pixel art without scaling its pixels. Reset before UI chrome. */
void display_origin(int x, int y);
void draw_pixel(int x, int y, int color);
void draw_line(int x0, int y0, int x1, int y1, int color);
/* Dots every step pixels from (x0, y0) along one row (y0 == y1) or one
   column (x0 == x1), up to (x1, y1). Requires x0 <= x1, y0 <= y1 and
   step >= 1; other calls draw nothing. Same pixels as a draw_pixel loop. */
void draw_dotted_line(int x0, int y0, int x1, int y1, int step, int color);
void draw_rect(int x, int y, int width, int height, int fill, int color);
/* Set (color 1) or clear the bits of one aligned 8-pixel byte, MSB leftmost.
   x is rounded down to a multiple of 8; off-screen bytes are ignored. */
void draw_byte(int x, int y, unsigned int bits, int color);
/* Size 0: 3x5 font in 4x6 cells. Size 1: 5x7. Size 2: doubled 5x7.
   Size 3: proportional 5-row UI font, glyphs 1..5 wide plus one space. */
#define TEXT_UI 3
void draw_text(int x, int y, const char *text, int size, int color);
/* Pixel width of text at a size, excluding trailing spacing. */
int text_width(const char *text, int size);
/* Text in color 1 with every pixel within one step of it cleared: the same
   pixels as drawing it in color 0 at the eight neighboring offsets first. */
void draw_text_outlined(int x, int y, const char *text, int size);
void draw_number(int x, int y, int value, int digits, int size, int color);

#endif
