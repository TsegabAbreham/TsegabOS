#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void pit_init(void) {
    uint32_t divisor = 1193180 / 1000;
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, (divisor >> 8) & 0xFF);
}


volatile uint64_t timer_ticks = 0;

void pit_handler() {
    timer_ticks++;

    outb(0x20, 0x20);
}

void sleep_ms(uint64_t milliseconds) {
    uint64_t end = timer_ticks + milliseconds;

    while (timer_ticks < end) {
        asm volatile ("hlt");
    }
}

