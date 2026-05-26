#include <stdint.h>

void mouse_init(uint32_t fb_width, uint32_t fb_height);
void mouse_render(void);


int mouse_get_x();
int mouse_get_y();
int mouse_left_pressed();