#include "block_ui.h"

void delay_render(const BlockUI *ui)
{
    block_ui_begin(ui);
    block_ui_text(18, BLOCK_UI_TOP + 2, "TIME");
    block_ui_text(62, BLOCK_UI_TOP + 2, "FEEDBACK");
    block_ui_text(126, BLOCK_UI_TOP + 2, "MIX");
    block_ui_control_knob(ui, 0, 28, 32);
    block_ui_control_knob(ui, 1, 80, 32);
    block_ui_control_knob(ui, 2, 132, 32);
    block_ui_param_text(ui, 0, 10, BLOCK_UI_BOTTOM - 7);
    block_ui_param_text(ui, 1, 68, BLOCK_UI_BOTTOM - 7);
    block_ui_param_text(ui, 2, 120, BLOCK_UI_BOTTOM - 7);
    block_ui_end(ui);
}
