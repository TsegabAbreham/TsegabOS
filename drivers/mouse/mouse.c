#include <stdint.h>

#include "../../GUI/bitmap/bitmap.h"
#include "../../drivers/Serial/libk/kprintf/kprintf.h"

static int mouse_x = 200;
static int mouse_y = 200;

static int prev_x = 200;
static int prev_y = 200;

static uint32_t screen_width;
static uint32_t screen_height;

static uint8_t mouse_cycle = 0;
static uint8_t mouse_packet[3];

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static void mouse_wait(uint8_t type) {
    uint32_t timeout = 100000;

    if (type == 0) {

        while (timeout--) {
            if (inb(0x64) & 1)
                return;
        }

    } else {

        while (timeout--) {
            if (!(inb(0x64) & 2))
                return;
        }
    }
}

static void mouse_write(uint8_t data) {
    mouse_wait(1);

    outb(0x64, 0xD4);

    mouse_wait(1);

    outb(0x60, data);
}

static uint8_t mouse_read(void) {
    mouse_wait(0);
    return inb(0x60);
}

extern uint8_t _binary_GUI_bitmap_bitmaps_Cursor_bmp_start[];
static bitmap_t cursor;

void mouse_init(uint32_t fb_width, uint32_t fb_height) {

    screen_width  = fb_width;
    screen_height = fb_height;

    uint8_t status;

    // enable auxiliary device
    mouse_wait(1);
    outb(0x64, 0xA8);

    // read command byte
    mouse_wait(1);
    outb(0x64, 0x20);

    mouse_wait(0);
    status = inb(0x60);

    // enable IRQ12
    status |= 0x02;

    // write command byte back
    mouse_wait(1);
    outb(0x64, 0x60);

    mouse_wait(1);
    outb(0x60, status);

    cursor = load_bmp(_binary_GUI_bitmap_bitmaps_Cursor_bmp_start);

    // enable packet streaming
    mouse_write(0xF4);

    // ACK
    mouse_read();

}

static uint8_t right_down = 0;
static uint8_t middle_down = 0;
static uint8_t left_down = 0;

void mouse_process_byte(uint8_t data) {

    if (mouse_cycle == 0 && !(data & 0x08)) {
        return;
    }

    mouse_packet[mouse_cycle++] = data;

    if (mouse_cycle < 3)
        return;

    mouse_cycle = 0;

    int dx = (int8_t)mouse_packet[1];
    int dy = (int8_t)mouse_packet[2];

    mouse_x += dx;
    mouse_y -= dy;

    // BUTTONS
    uint8_t buttons = mouse_packet[0];

    if (buttons & 0x01) {
        if (!left_down) {
            kprintf("Left click down\n");
        }
        left_down = 1;
    } else {
        if (left_down) {
            kprintf("Left click up\n");
        }
        left_down = 0;
    }

    if (buttons & 0x02){
        if (!right_down) {
            kprintf("Right click down\n");
        }
        right_down = 1;
    } else {
        if (right_down) {
            kprintf("Right click up\n");
        }
        right_down = 0;
    }

    if (buttons & 0x04){
        if (!middle_down) {
            kprintf("Middle click down\n");
        }
        middle_down = 1;
    } else {
        if (middle_down) {
            kprintf("Middle click up\n");
        }
        middle_down = 0;
    }



}

void mouse_handler(void) {

    uint8_t status = inb(0x64);

    if (!(status & 1))
        return;

    if (!(status & 0x20))
        return;

    uint8_t data = inb(0x60);

    mouse_process_byte(data);
}

int mouse_get_x()
{
    return mouse_x;
}

int mouse_get_y()
{
    return mouse_y;
}

int mouse_left_pressed()
{
    return left_down;
}