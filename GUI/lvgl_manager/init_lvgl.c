#include <stdint.h>
#include <stddef.h>

#include "../../external/external/lvgl/lvgl.h"
#include "../fb/framebuffer.h"
#include "../../memory/heap/heap.h"

// --------------------------------------------------
// CONTEXT (IMPORTANT)
// --------------------------------------------------
typedef struct {
    uint32_t *fb;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;   // in pixels, NOT bytes
} lvgl_ctx_t;

// --------------------------------------------------
// LVGL BUFFER
// --------------------------------------------------
static uint32_t lvgl_buf[800 * 50];

// --------------------------------------------------
// FLUSH CALLBACK
// --------------------------------------------------
void lvgl_flush(lv_display_t *disp,
                const lv_area_t *area,
                uint8_t *px_map)
{
    lvgl_ctx_t *ctx = (lvgl_ctx_t*)lv_display_get_user_data(disp);

    uint32_t *fb = ctx->fb;
    uint32_t pitch = ctx->pitch;

    uint32_t *src = (uint32_t*)px_map;

    for (int y = area->y1; y <= area->y2; y++) {

        uint32_t *dst = fb + y * pitch + area->x1;

        for (int x = area->x1; x <= area->x2; x++) {
            *dst++ = *src++;
        }
    }

    lv_display_flush_ready(disp);
}

// --------------------------------------------------
// INIT
// --------------------------------------------------
lv_display_t* LVGL_INIT(uint32_t *framebuffer,
                        uint32_t fb_width,
                        uint32_t fb_height,
                        uint32_t fb_pitch)
{
    lv_init();

    lv_display_t *disp = lv_display_create(fb_width, fb_height);

    // allocate context
    lvgl_ctx_t *ctx = kmalloc(sizeof(lvgl_ctx_t));
    ctx->fb = framebuffer;
    ctx->width = fb_width;
    ctx->height = fb_height;
    ctx->pitch = fb_pitch; // ensure pixels

    lv_display_set_user_data(disp, ctx);

    lv_display_set_buffers(
        disp,
        lvgl_buf,
        NULL,
        sizeof(lvgl_buf),
        LV_DISPLAY_RENDER_MODE_PARTIAL
    );

    lv_display_set_flush_cb(
        disp,
        lvgl_flush
    );

    return disp;
}