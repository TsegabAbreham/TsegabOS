#include "input.h"


void lvgl_mouse_read(lv_indev_t * indev,
                     lv_indev_data_t * data)
{
    (void)indev;

    data->point.x = mouse_get_x();
    data->point.y = mouse_get_y();

    if(mouse_left_pressed())
        data->state = LV_INDEV_STATE_PRESSED;
    else
        data->state = LV_INDEV_STATE_RELEASED;
}


lv_indev_t* lvgl_mouse_input_init(void) {
    lv_indev_t *mouse_indev =
        lv_indev_create();

    lv_indev_set_type(
        mouse_indev,
        LV_INDEV_TYPE_POINTER
    );

    lv_indev_set_read_cb(
        mouse_indev,
        lvgl_mouse_read
    );

    return mouse_indev;
}