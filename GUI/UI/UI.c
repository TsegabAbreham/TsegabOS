#include "../../external/external/lvgl/lvgl.h"
#include <stdint.h>

// ── Screen state ──────────────────────────────────────
static uint32_t screen_w = 1920;
static uint32_t screen_h = 1080;

// Call this once after LVGL init in kernel_main
void ui_init(uint32_t fb_width, uint32_t fb_height) {
    screen_w = fb_width;
    screen_h = fb_height;
}

// ── Scaling helpers ───────────────────────────────────
// All coordinates are designed for 1920x1080
// These scale them to whatever the actual resolution is
static uint32_t scale_x(uint32_t x) {
    return (x * screen_w) / 1920;
}

static uint32_t scale_y(uint32_t y) {
    return (y * screen_h) / 1080;
}

static uint32_t scale_w(uint32_t w) {
    return (w * screen_w) / 1920;
}

static uint32_t scale_h(uint32_t h) {
    return (h * screen_h) / 1080;
}

// ── Widgets ───────────────────────────────────────────
void create_label(uint32_t x, uint32_t y, char* text) {
    lv_obj_t* label = lv_label_create(lv_screen_active());

    lv_label_set_text(label, text);
    lv_obj_set_pos(label, scale_x(x), scale_y(y));
}

void create_button(uint32_t x, uint32_t y, uint32_t w, uint32_t h, char* text) {
    lv_obj_t* btn = lv_btn_create(lv_screen_active());

    lv_obj_set_size(btn, scale_w(w), scale_h(h));
    lv_obj_set_pos(btn, scale_x(x), scale_y(y));

    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_center(label);
}