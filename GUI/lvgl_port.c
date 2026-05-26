#include <stdint.h>
#include "lvgl.h"
#include "../../drivers/Serial/libk/tools/tools.h"

extern uint32_t* fb;
extern int fb_width;
extern int fb_height;
extern int fb_pitch;

/* Flush pixels to framebuffer */
void lvgl_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    for(int y = area->y1; y <= area->y2; y++) {
        uint32_t* dst = fb + y * fb_pitch + area->x1;
        uint32_t* src = (uint32_t*)px_map;

        memcpy(dst, src, (area->x2 - area->x1 + 1) * 4);
        px_map += (area->x2 - area->x1 + 1) * 4;
    }

    lv_display_flush_ready(disp);
}