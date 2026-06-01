#include "icon.h"
#include "../../../memory/heap/heap.h"

#define ICON_SIZE  48
#define ICON_W     72
#define ICON_H     72

static void drag_event_cb(lv_event_t *e) {
    lv_obj_t *cont = lv_event_get_target(e);
    lv_indev_t *indev = lv_indev_get_act();

    lv_point_t vect;
    lv_indev_get_vect(indev, &vect);

    lv_obj_set_pos(cont,
        lv_obj_get_x(cont) + vect.x,
        lv_obj_get_y(cont) + vect.y);
}

static void click_event_cb(lv_event_t *e) {
    desktop_icon_t *icon = (desktop_icon_t *)lv_event_get_user_data(e);

    if (icon->on_single_click)
        icon->on_single_click(icon);

    lv_obj_set_style_bg_color(icon->container, lv_color_hex(0x5B9BD5), 0);
    lv_obj_set_style_bg_opa(icon->container, LV_OPA_30, 0);
}

desktop_icon_t* icon_create(lv_obj_t *parent,
                            lv_image_dsc_t *image,
                            const char *name,
                            int x, int y,
                            void (*on_single_click)(desktop_icon_t *))
{
    desktop_icon_t *icon = kmalloc(sizeof(desktop_icon_t));
    icon->x               = x;
    icon->y               = y;
    icon->click_timer     = NULL;
    icon->on_single_click = on_single_click;

    // Container
    icon->container = lv_obj_create(parent);
    lv_obj_set_size(icon->container, ICON_W, ICON_H);
    lv_obj_set_pos(icon->container, x, y);
    lv_obj_set_style_bg_opa(icon->container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(icon->container, 0, 0);
    lv_obj_set_style_pad_all(icon->container, 0, 0);
    lv_obj_clear_flag(icon->container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(icon->container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(icon->container, LV_OBJ_FLAG_OVERFLOW_VISIBLE);

    lv_obj_add_event_cb(icon->container, drag_event_cb,  LV_EVENT_PRESSING, NULL);
    lv_obj_add_event_cb(icon->container, click_event_cb, LV_EVENT_CLICKED,  icon);

    // Image
    icon->img = lv_img_create(icon->container);
    lv_img_set_src(icon->img, image);
    lv_obj_set_size(icon->img, ICON_SIZE, ICON_SIZE);
    lv_obj_align(icon->img, LV_ALIGN_TOP_MID, 0, 0);

    // Label
    icon->label = lv_label_create(icon->container);
    lv_label_set_text(icon->label, name);
    lv_label_set_long_mode(icon->label, LV_LABEL_LONG_DOT);
    lv_obj_set_width(icon->label, ICON_W);
    lv_obj_set_style_text_color(icon->label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(icon->label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(icon->label, LV_ALIGN_BOTTOM_MID, 0, 0);

    return icon;
}

void icon_destroy(desktop_icon_t *icon) {
    if (!icon) return;
    if (icon->click_timer) lv_timer_del(icon->click_timer);
    lv_obj_del(icon->container);
    kfree(icon);
}