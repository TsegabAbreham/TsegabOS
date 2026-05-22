#include <stdint.h>

static uint32_t* fb;
static uint32_t fb_width;
static uint32_t fb_height;
static uint32_t fb_pitch;



void framebuffer_init(uint32_t *framebuffer, uint32_t framebuffer_width, uint32_t framebuffer_height,
     uint32_t framebuffer_pitch){
    fb = framebuffer;
    fb_width = framebuffer_width;
    fb_height = framebuffer_height;

    fb_pitch = framebuffer_pitch;
    fb_pitch = (fb_pitch/4);
}

void put_pixel(int x, int y, uint32_t color) {
    fb[y * fb_pitch + x] = color;
}

void clear_screen(uint32_t color){
    for (int y = 0; y < fb_height; y++){
        for (int x = 0; x < fb_width; x++){
            fb[y* fb_pitch + x] = color;
        }
    }
}


