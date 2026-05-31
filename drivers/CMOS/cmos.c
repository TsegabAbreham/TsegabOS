#include <stdint.h>
#include "cmos.h"

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

uint8_t cmos_read(uint8_t reg) {
    outb(0x70, reg);
    return inb(0x71);
}

void cmos_get_rtc(rtc_info_t *rtc) {
    rtc->second = cmos_read(SEC);
    rtc->minute = cmos_read(MIN);
    rtc->hour = cmos_read(HOUR);
    rtc->weekday = cmos_read(WEEKDAY);
    rtc->day_of_month = cmos_read(DAYOFMONTH);
    rtc->month = cmos_read(MONTH);
    rtc->year = cmos_read(YEAR);
    rtc->century = cmos_read(CENTURY);
}
