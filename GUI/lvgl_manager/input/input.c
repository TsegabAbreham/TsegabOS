#include "input.h"
#include "../../../drivers/keyboard/keyboard.h"

void lvgl_mouse_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    (void)indev;
    data->point.x = mouse_get_x();
    data->point.y = mouse_get_y();
    data->state = mouse_left_pressed()
        ? LV_INDEV_STATE_PRESSED
        : LV_INDEV_STATE_RELEASED;
}

// Track the current key being "held" for LVGL
static uint32_t current_key   = 0;
static uint8_t  key_consumed  = 1;   // 1 = nothing pending

void lvgl_keyboard_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    (void)indev;

    // If previous key hasn't been reported as released yet, release it now
    if (!key_consumed) {
        data->key   = current_key;
        data->state = LV_INDEV_STATE_RELEASED;
        key_consumed = 1;
        return;
    }

    // Try to get a new key from the buffer
    char c = kb_buf_pop();
    if (c == 0) {
        // Nothing pending — stay released
        data->key   = 0;
        data->state = LV_INDEV_STATE_RELEASED;
        return;
    }

    // Map special ASCII to LVGL key codes
    uint32_t lv_key;
    switch (c) {
        case '\b':  lv_key = LV_KEY_BACKSPACE; break;
        case '\n':
        case '\r':  lv_key = LV_KEY_ENTER;     break;
        case '\t':  lv_key = LV_KEY_NEXT;      break;  // tab = focus next
        default:    lv_key = (uint32_t)c;       break;
    }

    // Report as PRESSED — next call will release it
    current_key  = lv_key;
    key_consumed = 0;
    data->key    = lv_key;
    data->state  = LV_INDEV_STATE_PRESSED;
}

lv_indev_t* lvgl_mouse_input_init(void) {
    lv_indev_t *mouse_indev = lv_indev_create();
    lv_indev_set_type(mouse_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(mouse_indev, lvgl_mouse_read);
    return mouse_indev;
}

lv_indev_t* lvgl_keyboard_input_init(void) {
    lv_indev_t *keyboard_indev = lv_indev_create();
    lv_indev_set_type(keyboard_indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(keyboard_indev, lvgl_keyboard_read);
    return keyboard_indev;
}