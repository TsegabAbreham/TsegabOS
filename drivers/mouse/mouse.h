#include <stdint.h>

void mouse_init(void);
void mouse_poll(uint32_t fb_width, uint32_t fb_height);
void mouse_update(uint32_t fb_width, uint32_t fb_height);