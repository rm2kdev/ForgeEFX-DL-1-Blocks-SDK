#include "block_ui.h"

void phaser_render(const BlockUI *ui)
{
    block_ui_begin(ui);
    block_ui_text(12, BLOCK_UI_TOP + 3, "PHASER");
    block_ui_control_knob(ui, 0, 55, 36);
    block_ui_control_knob(ui, 1, 115, 36);
    block_ui_text(47, 46, "RATE");
    block_ui_text(105, 46, "DEPTH");
    block_ui_end(ui);
}
