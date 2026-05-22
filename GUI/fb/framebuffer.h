#include <stdint.h>

void framebuffer_init(uint32_t *framebuffer, uint32_t framebuffer_width, uint32_t framebuffer_height,
     uint32_t framebuffer_pitch);

void put_pixel(int x, int y, uint32_t color);
void clear_screen(uint32_t color);
void draw_rect(int x, int y, int w, int h, uint32_t color);
