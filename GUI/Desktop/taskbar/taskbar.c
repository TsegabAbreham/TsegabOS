#include "taskbar.h"

#define TASKBAR_HEIGHT  44
#define TASKBAR_BG      0x1a1a1a

static lv_obj_t  *clock_label;
static lv_timer_t *clock_timer;

// Addis Ababa is UTC+3
#define TIMEZONE_OFFSET 3

static void clock_update_cb(lv_timer_t *t) {
    (void)t;

    rtc_info_t rtc;
    cmos_get_rtc(&rtc);

    uint8_t hour   = rtc.hour;
    uint8_t minute = rtc.minute;

    // Apply timezone
    hour = (hour + TIMEZONE_OFFSET) % 24;

    char buf[6];
    buf[0] = '0' + (hour   / 10);
    buf[1] = '0' + (hour   % 10);
    buf[2] = ':';
    buf[3] = '0' + (minute / 10);
    buf[4] = '0' + (minute % 10);
    buf[5] = '\0';

    lv_label_set_text(clock_label, buf);
}

void taskbar_init(lv_obj_t *parent, uint32_t screen_width,
                  uint32_t screen_height)
{
    // Bar
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_size(bar, screen_width, TASKBAR_HEIGHT);
    lv_obj_set_pos(bar, 0, screen_height - TASKBAR_HEIGHT);
    lv_obj_set_style_bg_color(bar, lv_color_hex(TASKBAR_BG), 0);
    lv_obj_set_style_border_side(bar, LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_border_color(bar, lv_color_hex(0x333333), 0);
    lv_obj_set_style_border_width(bar, 1, 0);
    lv_obj_set_style_radius(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 0, 0);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);

    // Three circles (like a dock placeholder)
    uint32_t colors[3] = { 0xFF5F57, 0xFFBD2E, 0x28C840 };
    for (int i = 0; i < 3; i++) {
        lv_obj_t *circle = lv_obj_create(bar);
        lv_obj_set_size(circle, 16, 16);
        lv_obj_set_pos(circle, 12 + i * 24, (TASKBAR_HEIGHT - 16) / 2);
        lv_obj_set_style_bg_color(circle, lv_color_hex(colors[i]), 0);
        lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(circle, 0, 0);
        lv_obj_set_style_pad_all(circle, 0, 0);
    }

    // Clock on the right
    clock_label = lv_label_create(bar);
    lv_label_set_text(clock_label, "00:00");
    lv_obj_set_style_text_color(clock_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(clock_label, LV_ALIGN_RIGHT_MID, -14, 0);

    clock_timer = lv_timer_create(clock_update_cb, 1000, NULL);
    clock_update_cb(NULL);
}

uint32_t taskbar_get_height(void) {
    return TASKBAR_HEIGHT;
}