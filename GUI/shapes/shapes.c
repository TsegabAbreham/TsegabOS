#include <stdint.h>
#include "../fb/framebuffer.h"

void draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int yy = y; yy < y + h; yy++) {
        for (int xx = x; xx < x + w; xx++) {
            put_pixel(xx, yy, color);
        }
    }
}
