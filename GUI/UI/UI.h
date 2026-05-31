#ifndef UI_H
#define UI_H

#include <stdint.h>

void ui_init(uint32_t fb_width, uint32_t fb_height);
void create_label(uint32_t x, uint32_t y, char* text);
void create_button(uint32_t x, uint32_t y, uint32_t w, uint32_t h, char* text);

#endif