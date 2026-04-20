#include "sceiostat_mock.h"
#define RGBA8(r,g,b,a) 0
typedef void vita2d_texture;
typedef void vita2d_font;
#define RGBA8(r,g,b,a) 0
int vita2d_font_text_height(void*, int, const char*);
int vita2d_font_text_width(void*, int, const char*);
void vita2d_start_drawing();
void vita2d_clear_screen();
void vita2d_end_drawing();
void vita2d_swap_buffers();
void vita2d_wait_rendering_done();
void vita2d_draw_rectangle(float, float, float, float, unsigned int);
void vita2d_draw_texture(vita2d_texture*, float, float);
void vita2d_font_draw_text(vita2d_font*, float, float, unsigned int, int, const char*);
struct SceIoStat {};
int sceIoGetstat(const char*, struct SceIoStat*);
int sceIoMkdir(const char*, int);
