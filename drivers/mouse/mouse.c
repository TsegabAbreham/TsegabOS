#include <stdint.h>
#include "../../drivers/Serial/libk/kprintf/kprintf.h"
#include "../../GUI/shapes/shapes.h"

static int mouse_x = 200;
static int mouse_y = 200;
static int prev_x = 200;
static int prev_y = 200;

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

void mouse_init(void) {
    uint8_t status;

    mouse_wait(1);
    outb(0x64, 0xA8);          // enable auxiliary mouse device

    mouse_wait(1);
    outb(0x64, 0x20);          // read controller command byte
    mouse_wait(0);
    status = inb(0x60);

    status |= 0x02;            // enable IRQ12 bit in command byte

    mouse_wait(1);
    outb(0x64, 0x60);          // write command byte
    mouse_wait(1);
    outb(0x60, status);

    mouse_write(0xF4);         // enable data reporting
    (void)mouse_read();        // consume ACK
}

static uint8_t mouse_cycle = 0;
static uint8_t mouse_packet[3];

static void mouse_process_byte(uint8_t data, uint32_t fb_width, uint32_t fb_height) {
    if (mouse_cycle == 0 && !(data & 0x08)) {
        return;
    }

    mouse_packet[mouse_cycle++] = data;

    if (mouse_cycle == 3) {
        mouse_cycle = 0;

        int dx = (int8_t)mouse_packet[1];
        int dy = (int8_t)mouse_packet[2];

        mouse_x += dx;
        mouse_y -= dy;

        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        if (mouse_x > (int)fb_width - 50) mouse_x = (int)fb_width - 50;
        if (mouse_y > (int)fb_height - 50) mouse_y = (int)fb_height - 50;
    }
}


void mouse_poll(uint32_t fb_width, uint32_t fb_height) {
    while (inb(0x64) & 1) {
        uint8_t status = inb(0x64);
        uint8_t data = inb(0x60);

        if (status & 0x20) {
            mouse_process_byte(data, fb_width, fb_height);
            kprintf(" %d \n", (int)data);
        }

    }
}

void mouse_update(uint32_t fb_width, uint32_t fb_height){
    draw_circle(mouse_x, mouse_y, 10, 0xFF0000);
    while (1) {
        mouse_poll(fb_width, fb_height);

        if (mouse_x != prev_x || mouse_y != prev_y) {
            draw_circle(prev_x, prev_y, 10, 0x202020);
            draw_circle(mouse_x, mouse_y, 10, 0xFF0000);
            prev_x = mouse_x;
            prev_y = mouse_y;
        }
    }
}