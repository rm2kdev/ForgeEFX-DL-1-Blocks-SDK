#ifndef FREEFX_BLOCK_UI_H
#define FREEFX_BLOCK_UI_H

#include "display.h"

typedef struct BlockUI {
    int block;
    int effect;
    const int *params;
    int page; /* Authored artwork section (selected_param / 4), not LCD paging. */
    int selected_param;
    int enabled;
    int input_peak; /* This slot's routed input peak, absolute Q16. */
    int output_peak; /* This slot's output peak, absolute Q16. */
    const int *input_wave; /* 128 captured Q16 points; null only for offline previews. */
    const int *output_wave;
    int wave_divisor; /* Shared input/output display range, Q16; zero means full scale. */
    /* Animation context. time: continuous 10 ms ticks at the original idle speed.
       edit_param: last edited parameter in this slot, or -1. edit_age: ticks since
       that edit (saturates at 10000). edit_dir: +1 or -1 for the last step.
       level: this slot's absolute Q16 output peak (65536 is full scale). */
    int time;
    int edit_param;
    int edit_age;
    int edit_dir;
    int level;
    const char *model_name; /* Native NAM filename/status, supplied by the panel. */
    int load_status; /* 0 ready, 1 preparing a large history, 2 allocation failed. */
} BlockUI;

#define BLOCK_UI_TICKS_PER_SECOND 100
/* Block drawings use this logical canvas at every physical LCD resolution.
   The shared renderer uniformly fits rows 11..55 into the content area, keeping
   pixel art, fonts and overlays together. Chrome and input use ScreenConfig.h. */
#define BLOCK_UI_WIDTH 160
#define BLOCK_UI_HEIGHT 80
#define BLOCK_UI_TOP 12
#define BLOCK_UI_BOTTOM 54
#define BLOCK_UI_FOOTER UI_PARAM_FOOTER
/* An edit keeps the artwork's emphasis on its parameter for this many ticks. */
#define BLOCK_UI_EDIT_HOLD 150

/* Required paired lifecycle for block-owned edit screens. Begin draws the LCD
   header and selects a cleared logical canvas; end composites the canvas and
   draws the responsive controls. Draw only between these calls, without clearing
   or switching the target. Sequential message-thread use; never audio callbacks.
   The controller owns LCD transfers. */
void block_ui_begin(const BlockUI *ui);
void block_ui_end(const BlockUI *ui);
int block_ui_clamp(int value, int low, int high);
/* Registry-formatted value, e.g. "6.5", "+4DB", "380MS", "1.6K", "FAST".
   text must hold at least 16 characters. */
void block_ui_format(char *text, int effect, int parameter, int value);
/* Number in the given font size with its unit; size 2 sets the unit smaller. */
void block_ui_value(int x, int y, int effect, int parameter, int value, int size);
/* Integer sine for drawing: phase 0..1023 is one cycle; returns -256..256. */
int block_ui_sine(int phase);
/* Rotary knob, 270-degree sweep from lower left. percent 0..100.
   active draws a filled cap (for the knob being edited). */
void block_ui_knob(int cx, int cy, int radius, int percent, int active);
/* Compact parameter knob for pedal faces, normalized to registry bounds.
   A steady underline identifies the selected control. */
void block_ui_control_knob(const BlockUI *ui, int parameter, int cx, int cy);

/* Interaction context. */
/* Parameter position within its registry bounds, 0..1000. */
int block_ui_norm(const BlockUI *ui, int parameter);
/* Parameter the artwork should emphasize: the one just edited, else the selected one. */
int block_ui_focus(const BlockUI *ui);
/* Nonzero while this parameter was edited within BLOCK_UI_EDIT_HOLD ticks. */
int block_ui_editing(const BlockUI *ui, int parameter);
/* Animation ticks that hold still while the block is bypassed. */
int block_ui_clock(const BlockUI *ui);
/* Output activity, 0..1000, with a -60 dB floor and a compressed level scale.
   Bypass and unavailable DSP have no activity. Does not alter audio parameters. */
int block_ui_signal(const BlockUI *ui);
/* Captured trace at position 0..127, scaled to the caller's drawing units.
   The fallback is used only by offline artwork previews without a capture. */
int block_ui_trace(const BlockUI *ui, int output, int position, int scale, int fallback);
/* Modest input-driven expansion of an existing animation detail; preserves idle art. */
int block_ui_react(const BlockUI *ui, int value);

/* Text in the 5-row UI font. Right-aligned variants end at column right. */
void block_ui_text(int x, int y, const char *text);
void block_ui_text_right(int right, int y, const char *text);
/* "NAME VALUE" for a parameter; inverted while that parameter is being edited. */
void block_ui_readout(const BlockUI *ui, int parameter, int x, int y);
void block_ui_readout_right(const BlockUI *ui, int parameter, int right, int y);
/* Formatted value alone. */
void block_ui_param_text(const BlockUI *ui, int parameter, int x, int y);
void block_ui_param_text_right(const BlockUI *ui, int parameter, int right, int y);

/* Drawing helpers. */
void block_ui_dots_h(int x0, int x1, int y, int step);
void block_ui_dots_v(int x, int y0, int y1, int step);
/* Checkerboard shading of rows y0..y1 in one column (either order). */
void block_ui_stipple(int x, int y0, int y1);
void block_ui_stipple_rect(int x, int y, int width, int height);
/* Curve column: line from the previous column's y, shaded down to base. */
void block_ui_area(int x, int y, int last_y, int base);
/* Dotted square with centre axes for transfer plots (size pixels square). */
void block_ui_plot_frame(int x, int y, int size);
/* Transfer curve inside a plot frame. drive, level, mix are 0..1000; asym
   -1000..1000 squashes the negative half; hardness 0 soft, 1 warm, 2 hard, 3 fuzz. */
void block_ui_transfer(int x, int y, int size, int drive, int hardness, int asym, int level, int mix);
/* Shaded tone/frequency response over columns x0..x1, log-like left to right.
   Gains are -1000..1000 (full swing is half the height between top and base);
   mid is a bell at mid_pos 0..1000 whose width is 50..1000. The flat line
   sits midway between top and base. */
void block_ui_response(int x0, int x1, int top, int base, int low, int mid, int mid_pos, int width, int high);
/* Inverted pill ending at column right: black text on white. */
void block_ui_badge(int right, int y, const char *text);
/* Drive-face furniture: two description lines, ON/OFF pill, and key hint.
   Artwork to the left of column 50 belongs to the caller. */
void block_ui_face(const BlockUI *ui, const char *line1, const char *line2);
/* EQ band from zero_y to level_y; the active band is solid, others shaded. */
void block_ui_band(int x0, int x1, int zero_y, int level_y, int active);
/* Horizontal position ruler with ticks and a marker at permille 0..1000. */
void block_ui_ruler(int x0, int x1, int y, int permille);
/* Segmented meter: solid segments up to permille, shaded beyond. */
void block_ui_gauge(int x, int y, int width, int height, int permille);

#endif
