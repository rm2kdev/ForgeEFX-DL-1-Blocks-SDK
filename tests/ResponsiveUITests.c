#include "block_ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void responsive_ui_render(const BlockUI *ui);

static int rendering, begins, ends, traces, segments, focus_lines;
static int knob_percent, knob_active, meter, last_trace;
static const BlockUI *current;

static void check(int condition)
{
    if (!condition) {
        fputs("Responsive UI contract failed\n", stderr);
        exit(1);
    }
}

static void point(int x, int y)
{
    check(rendering && x >= 0 && x < BLOCK_UI_WIDTH);
    check(y >= BLOCK_UI_TOP && y <= BLOCK_UI_BOTTOM);
}

void block_ui_begin(const BlockUI *ui)
{
    check(!rendering);
    current = ui;
    rendering = 1;
    ++begins;
}

void block_ui_end(const BlockUI *ui)
{
    check(rendering && current == ui);
    rendering = 0;
    ++ends;
}

int block_ui_clamp(int value, int low, int high)
{
    return value < low ? low : value > high ? high : value;
}

int block_ui_norm(const BlockUI *ui, int parameter)
{
    check(rendering && parameter == 0);
    return ui->params[parameter] * 5;
}

int block_ui_editing(const BlockUI *ui, int parameter)
{
    return ui->edit_param == parameter && ui->edit_age < BLOCK_UI_EDIT_HOLD;
}

int block_ui_focus(const BlockUI *ui)
{
    return block_ui_editing(ui, 0) ? ui->edit_param : ui->selected_param;
}

int block_ui_signal(const BlockUI *ui)
{
    return ui->enabled && !ui->load_status ? ui->level : 0;
}

int block_ui_trace(const BlockUI *ui, int output, int position, int scale, int fallback)
{
    check(rendering && ui == current && output == 1);
    check(position >= 0 && position <= 127 && position >= last_trace);
    check(scale > 0 && fallback == 0);
    last_trace = position;
    ++traces;
    /* Deliberately return overshoot: the demo must contain captured transients. */
    return ui->output_wave ? (position % 2 ? 1000 : -1000) : fallback;
}

void draw_line(int x0, int y0, int x1, int y1, int color)
{
    point(x0, y0);
    point(x1, y1);
    check(color == 1);
    if (x0 >= 64) {
        ++segments;
        if (!current->enabled || current->load_status || !current->output_wave)
            check(y0 == BLOCK_UI_TOP + 20 && y1 == y0);
    } else {
        ++focus_lines;
    }
}

void draw_rect(int x, int y, int width, int height, int fill, int color)
{
    point(x, y);
    point(x + width - 1, y + height - 1);
    check(fill == 0 && color == 1);
}

void block_ui_dots_h(int x0, int x1, int y, int step)
{
    point(x0, y);
    point(x1, y);
    check(step > 0);
}

void block_ui_text(int x, int y, const char *text)
{
    point(x, y);
    point(x + (int)strlen(text) * 6 - 1, y + 4);
}

void block_ui_readout(const BlockUI *ui, int parameter, int x, int y)
{
    check(ui == current && parameter == 0);
    point(x, y);
}

void block_ui_knob(int x, int y, int radius, int percent, int active)
{
    point(x - radius, y - radius);
    point(x + radius, y + radius);
    check(percent >= 0 && percent <= 100);
    knob_percent = percent;
    knob_active = active;
}

void block_ui_gauge(int x, int y, int width, int height, int permille)
{
    point(x, y);
    point(x + width - 1, y + height - 1);
    meter = permille;
}

int main(void)
{
    int params[64] = {100};
    int wave[128] = {0};
    BlockUI ui = {0};
    ui.params = params;
    ui.edit_param = -1;
    for (int capture = 0; capture <= 1; ++capture) {
        ui.output_wave = capture ? wave : NULL;
        for (int enabled = 0; enabled <= 1; ++enabled) {
            ui.enabled = enabled;
            for (int loading = 0; loading <= 2; ++loading) {
                ui.load_status = loading;
                for (int gain = 0; gain <= 200; gain += 100) {
                    params[0] = gain;
                    for (int edited = 0; edited <= 2; ++edited) {
                        BlockUI before;
                        ui.edit_param = edited ? 0 : -1;
                        ui.edit_age = edited == 2 ? BLOCK_UI_EDIT_HOLD : 0;
                        ui.level = gain * 5;
                        before = ui;
                        traces = segments = focus_lines = 0;
                        last_trace = -1;
                        responsive_ui_render(&ui);
                        check(!rendering && begins == ends);
                        check(memcmp(&ui, &before, sizeof(ui)) == 0 && params[0] == gain);
                        check(knob_percent == gain / 2 && knob_active == (edited == 1));
                        check(focus_lines == 1 && segments > 0);
                        check(meter == (enabled && !loading ? ui.level : 0));
                        check(enabled && !loading ? traces > 0 && last_trace == 127 : traces == 0);
                    }
                }
            }
        }
    }
    return 0;
}
