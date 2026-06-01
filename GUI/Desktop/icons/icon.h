#ifndef ICON_H
#define ICON_H

#include "../../../external/external/lvgl/lvgl.h"

typedef struct desktop_icon_t {
    lv_obj_t        *container;
    lv_obj_t        *img;
    lv_obj_t        *label;
    lv_timer_t      *click_timer;
    int              x, y;
    void           (*on_single_click)(struct desktop_icon_t *);
} desktop_icon_t;

desktop_icon_t* icon_create(lv_obj_t *parent,
                            lv_image_dsc_t *image,
                            const char *name,
                            int x, int y,
                            void (*on_single_click)(desktop_icon_t *));

void icon_destroy(desktop_icon_t *icon);

#endif