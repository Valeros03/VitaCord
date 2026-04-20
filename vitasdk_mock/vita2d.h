#ifndef _VITA2D_H_
#define _VITA2D_H_
typedef struct vita2d_font vita2d_font;
typedef struct vita2d_texture vita2d_texture;
#define RGBA8(r,g,b,a) 0
int vita2d_texture_get_width(const vita2d_texture *texture);
int vita2d_font_text_width(vita2d_font *font, int size, const char *text);
int vita2d_font_text_height(vita2d_font *font, int size, const char *text);
void vita2d_draw_rectangle(float x, float y, float w, float h, unsigned int color);
void vita2d_font_draw_text(vita2d_font *font, int x, int y, unsigned int color, int size, const char *text);
void vita2d_draw_texture(vita2d_texture *texture, float x, float y);
void vita2d_draw_texture_rotate(vita2d_texture *texture, float x, float y, float angle);
void vita2d_draw_texture_part_scale(vita2d_texture *texture, float x, float y, float tex_x, float tex_y, float tex_w, float tex_h, float x_scale, float y_scale);
void vita2d_start_drawing();
void vita2d_clear_screen();
void vita2d_end_drawing();
void vita2d_swap_buffers();
void vita2d_wait_rendering_done();
void vita2d_init();
void vita2d_set_clear_color(unsigned int color);
void vita2d_fini();
void vita2d_free_texture(vita2d_texture *texture);
void vita2d_free_font(vita2d_font *font);
vita2d_font *vita2d_load_font_file(const char *filename);
vita2d_texture *vita2d_load_PNG_file(const char *filename);
#endif
