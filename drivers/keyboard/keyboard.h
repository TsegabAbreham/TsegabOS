#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

void keyboard_init(void);
void keyboard_handler(void);   // IRQ handler — call from IDT stub
char kb_buf_pop(void);         // poll from LVGL input callback

#endif