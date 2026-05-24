#include <stdint.h>

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t* pixels; 
} bitmap_t;

void draw_bitmap(int x, int y, bitmap_t* bmp);
bitmap_t load_bmp(const uint8_t* data);