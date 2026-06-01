#ifndef TASKBAR_H
#define TASKBAR_H

#include <stdint.h>
#include "../../../external/external/lvgl/lvgl.h"
#include "../../../drivers/CMOS/cmos.h"

void     taskbar_init(lv_obj_t *parent, uint32_t screen_width,
                      uint32_t screen_height);
uint32_t taskbar_get_height(void);

#endif