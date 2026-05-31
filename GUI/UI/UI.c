#include "../../external/external/lvgl/lvgl.h"
#include <stdint.h>


void map(int ) {

}

void create_label(uint32_t x, uint32_t y, char *text) {
    lv_obj_t *label = lv_label_create(lv_scr_act());

    lv_label_set_text(label, text);
    lv_obj_set_pos(label, x, y);

}


void create_button(uint32_t x, uint32_t y, uint32_t w, uint32_t h, char *text) {
    lv_obj_t *btn =
        lv_btn_create(lv_screen_active());
    lv_obj_set_size(btn, w, h);

    lv_obj_align(
        btn,
        LV_ALIGN_DEFAULT,
        x,
        y
    );

    lv_obj_t *label =
        lv_label_create(btn);

    lv_label_set_text(
        label,
        text
    );

    lv_obj_center(label);
}
