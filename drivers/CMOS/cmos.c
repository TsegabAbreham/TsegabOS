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

// Convert BCD byte to normal decimal
static uint8_t bcd_to_dec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

void cmos_get_rtc(rtc_info_t *rtc) {
    rtc->second = bcd_to_dec(cmos_read(SEC));
    rtc->minute = bcd_to_dec(cmos_read(MIN));
    rtc->hour = bcd_to_dec(cmos_read(HOUR));
    rtc->weekday = bcd_to_dec(cmos_read(WEEKDAY));
    rtc->day_of_month = bcd_to_dec(cmos_read(DAYOFMONTH));
    rtc->month = bcd_to_dec(cmos_read(MONTH));
    rtc->year = bcd_to_dec(cmos_read(YEAR));
    rtc->century = bcd_to_dec(cmos_read(CENTURY));
}
