#ifndef RETRO_SCREEN_CONFIG_H
#define RETRO_SCREEN_CONFIG_H

/* Physical LCD pixels. Override together through CMake or change these defaults.
   Chrome fonts and parameter cells retain their pixel size on a larger LCD;
   block artwork uses the shared logical canvas and scales to the content area. */
#ifndef RETRO_LCD_WIDTH
#define RETRO_LCD_WIDTH 160
#endif
#ifndef RETRO_LCD_HEIGHT
#define RETRO_LCD_HEIGHT 110
#endif
#if RETRO_LCD_WIDTH < 160 || RETRO_LCD_HEIGHT < 80
#error ForgeEFX DL-1 requires an LCD of at least 160x80 pixels
#endif
#define DISPLAY_WIDTH RETRO_LCD_WIDTH
#define DISPLAY_HEIGHT RETRO_LCD_HEIGHT
#define DISPLAY_STRIDE ((DISPLAY_WIDTH + 7) / 8)
#define DISPLAY_BYTES (DISPLAY_STRIDE * DISPLAY_HEIGHT)

/* Shared geometry for rendering, paging and pointer input. */
#define UI_PARAM_COLUMNS 4
/* Retain one-row compatibility for explicitly requested legacy LCDs. */
#define UI_PARAM_ROWS (DISPLAY_HEIGHT >= 104 ? 2 : 1)
#define UI_PARAM_ROW_HEIGHT 24
#define UI_PARAM_PAGE_SIZE (UI_PARAM_COLUMNS * UI_PARAM_ROWS)
#define UI_PARAM_X(column) ((column) * DISPLAY_WIDTH / UI_PARAM_COLUMNS)
#define UI_PARAM_FOOTER (DISPLAY_HEIGHT - UI_PARAM_ROWS * UI_PARAM_ROW_HEIGHT)
#define UI_PARAM_Y(row) (UI_PARAM_FOOTER + (row) * UI_PARAM_ROW_HEIGHT)
#define UI_LIST_ROWS ((DISPLAY_HEIGHT - 26) / 9)
#define UI_PRESET_ROWS ((DISPLAY_HEIGHT - 25) / 11)
#define UI_GRID_COLUMNS ((DISPLAY_WIDTH - 6) / 22)
#define UI_GRID_ROWS 6
#define UI_GRID_VISIBLE_ROWS (((DISPLAY_HEIGHT - 20) / 15) < UI_GRID_ROWS ? ((DISPLAY_HEIGHT - 20) / 15) : UI_GRID_ROWS)
#define UI_GRID_TOP (12 + (DISPLAY_HEIGHT - 20 - UI_GRID_VISIBLE_ROWS * 15) / 2)

#endif
