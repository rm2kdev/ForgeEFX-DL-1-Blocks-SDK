#include "forgeefx_block.h"
#if defined(_MSC_VER)
#define FORGEEFX_TLS __declspec(thread)
#else
#define FORGEEFX_TLS _Thread_local
#endif
static FORGEEFX_TLS const ForgeEFXRenderServices *current_services;
const ForgeEFXRenderServices *forgeefx_set_render_services(const ForgeEFXRenderServices *services) { const ForgeEFXRenderServices *previous = current_services; current_services = services; return previous; }
void draw_pixel(int x, int y, int color) { current_services->draw_pixel(x,y,color); }
void draw_line(int x0, int y0, int x1, int y1, int color) { current_services->draw_line(x0,y0,x1,y1,color); }
void draw_dotted_line(int x0, int y0, int x1, int y1, int step, int color) { current_services->draw_dotted_line(x0,y0,x1,y1,step,color); }
void draw_rect(int x, int y, int width, int height, int fill, int color) { current_services->draw_rect(x,y,width,height,fill,color); }
void draw_byte(int x, int y, unsigned int bits, int color) { current_services->draw_byte(x,y,bits,color); }
void draw_text(int x, int y, const char *text, int size, int color) { current_services->draw_text(x,y,text,size,color); }
int text_width(const char *text, int size) { return current_services->text_width(text,size); }
void draw_text_outlined(int x, int y, const char *text, int size) { current_services->draw_text_outlined(x,y,text,size); }
void draw_number(int x, int y, int value, int digits, int size, int color) { current_services->draw_number(x,y,value,digits,size,color); }
void block_ui_begin(const BlockUI *ui) { current_services->block_ui_begin(ui); }
void block_ui_end(const BlockUI *ui) { current_services->block_ui_end(ui); }
int block_ui_clamp(int value, int low, int high) { return current_services->block_ui_clamp(value,low,high); }
void block_ui_format(char *text, int effect, int parameter, int value) { current_services->block_ui_format(text,effect,parameter,value); }
void block_ui_value(int x, int y, int effect, int parameter, int value, int size) { current_services->block_ui_value(x,y,effect,parameter,value,size); }
int block_ui_sine(int phase) { return current_services->block_ui_sine(phase); }
void block_ui_knob(int cx, int cy, int radius, int percent, int active) { current_services->block_ui_knob(cx,cy,radius,percent,active); }
void block_ui_control_knob(const BlockUI *ui, int parameter, int cx, int cy) { current_services->block_ui_control_knob(ui,parameter,cx,cy); }
int block_ui_norm(const BlockUI *ui, int parameter) { return current_services->block_ui_norm(ui,parameter); }
int block_ui_focus(const BlockUI *ui) { return current_services->block_ui_focus(ui); }
int block_ui_editing(const BlockUI *ui, int parameter) { return current_services->block_ui_editing(ui,parameter); }
int block_ui_clock(const BlockUI *ui) { return current_services->block_ui_clock(ui); }
int block_ui_signal(const BlockUI *ui) { return current_services->block_ui_signal(ui); }
int block_ui_trace(const BlockUI *ui, int output, int position, int scale, int fallback) { return current_services->block_ui_trace(ui,output,position,scale,fallback); }
int block_ui_react(const BlockUI *ui, int value) { return current_services->block_ui_react(ui,value); }
void block_ui_text(int x, int y, const char *text) { current_services->block_ui_text(x,y,text); }
void block_ui_text_right(int right, int y, const char *text) { current_services->block_ui_text_right(right,y,text); }
void block_ui_readout(const BlockUI *ui, int parameter, int x, int y) { current_services->block_ui_readout(ui,parameter,x,y); }
void block_ui_readout_right(const BlockUI *ui, int parameter, int right, int y) { current_services->block_ui_readout_right(ui,parameter,right,y); }
void block_ui_param_text(const BlockUI *ui, int parameter, int x, int y) { current_services->block_ui_param_text(ui,parameter,x,y); }
void block_ui_param_text_right(const BlockUI *ui, int parameter, int right, int y) { current_services->block_ui_param_text_right(ui,parameter,right,y); }
void block_ui_dots_h(int x0, int x1, int y, int step) { current_services->block_ui_dots_h(x0,x1,y,step); }
void block_ui_dots_v(int x, int y0, int y1, int step) { current_services->block_ui_dots_v(x,y0,y1,step); }
void block_ui_stipple(int x, int y0, int y1) { current_services->block_ui_stipple(x,y0,y1); }
void block_ui_stipple_rect(int x, int y, int width, int height) { current_services->block_ui_stipple_rect(x,y,width,height); }
void block_ui_area(int x, int y, int last_y, int base) { current_services->block_ui_area(x,y,last_y,base); }
void block_ui_plot_frame(int x, int y, int size) { current_services->block_ui_plot_frame(x,y,size); }
void block_ui_transfer(int x, int y, int size, int drive, int hardness, int asym, int level, int mix) { current_services->block_ui_transfer(x,y,size,drive,hardness,asym,level,mix); }
void block_ui_response(int x0, int x1, int top, int base, int low, int mid, int mid_pos, int width, int high) { current_services->block_ui_response(x0,x1,top,base,low,mid,mid_pos,width,high); }
void block_ui_badge(int right, int y, const char *text) { current_services->block_ui_badge(right,y,text); }
void block_ui_face(const BlockUI *ui, const char *line1, const char *line2) { current_services->block_ui_face(ui,line1,line2); }
void block_ui_band(int x0, int x1, int zero_y, int level_y, int active) { current_services->block_ui_band(x0,x1,zero_y,level_y,active); }
void block_ui_ruler(int x0, int x1, int y, int permille) { current_services->block_ui_ruler(x0,x1,y,permille); }
void block_ui_gauge(int x, int y, int width, int height, int permille) { current_services->block_ui_gauge(x,y,width,height,permille); }
