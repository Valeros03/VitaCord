#ifndef _VITA2D_H_
#define _VITA2D_H_
typedef struct vita2d_texture vita2d_texture;
typedef struct vita2d_font vita2d_font;
#define RGBA8(r,g,b,a) (((a)<<24)|((b)<<16)|((g)<<8)|(r))
int vita2d_font_text_height(vita2d_font* font, int size, const char* text);
int vita2d_font_text_width(vita2d_font* font, int size, const char* text);
void vita2d_font_draw_text(vita2d_font* font, int x, int y, unsigned int color, int size, const char* text);
void vita2d_draw_texture(vita2d_texture* tex, float x, float y);
void vita2d_draw_texture_rotate(vita2d_texture* tex, float x, float y, float rad);
void vita2d_draw_rectangle(float x, float y, float w, float h, unsigned int color);
void vita2d_start_drawing();
void vita2d_end_drawing();
void vita2d_swap_buffers();
void vita2d_clear_screen();
void vita2d_wait_rendering_done();
#endif
