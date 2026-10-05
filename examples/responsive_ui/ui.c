#include "block_ui.h"

/* The host fits this logical artwork to the physical display and draws the
   responsive parameter controls in block_ui_end. Never size from DISPLAY_WIDTH
   or draw over the host's header/footer inside the artwork canvas. */
void responsive_ui_render(const BlockUI *ui)
{
    const int left = 64;
    const int right = BLOCK_UI_WIDTH - 8;
    const int center = BLOCK_UI_TOP + 20;
    const int amplitude = 9;
    int last_y = center;
    int x;

    block_ui_begin(ui);

    block_ui_readout(ui, 0, 5, BLOCK_UI_TOP + 2);
    block_ui_knob(26, center + 2, 12, block_ui_norm(ui, 0) / 10,
                  block_ui_editing(ui, 0));
    if (block_ui_focus(ui) == 0) {
        draw_line(14, BLOCK_UI_BOTTOM - 3, 38, BLOCK_UI_BOTTOM - 3, 1);
    }

    block_ui_text(left, BLOCK_UI_TOP + 2, "OUTPUT");
    draw_rect(left - 2, center - amplitude - 2,
              right - left + 5, amplitude * 2 + 5, 0, 1);
    block_ui_dots_h(left, right, center, 4);
    for (x = left; x <= right; ++x) {
        int position = (x - left) * 127 / (right - left);
        /* No invented activity: missing offline captures fall back to silence.
           Bypass/load overlays are supplied by the host in block_ui_end. */
        int trace = ui->enabled && !ui->load_status
            ? block_ui_trace(ui, 1, position, amplitude, 0) : 0;
        int y = center - block_ui_clamp(trace, -amplitude, amplitude);
        if (x != left) draw_line(x - 1, last_y, x, y, 1);
        last_y = y;
    }

    block_ui_text(left, BLOCK_UI_BOTTOM - 6, "LVL");
    block_ui_gauge(left + 22, BLOCK_UI_BOTTOM - 6,
                   right - left - 21, 5, block_ui_signal(ui));
    block_ui_end(ui);
}
