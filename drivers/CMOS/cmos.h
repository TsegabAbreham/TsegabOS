#include <stdint.h>



#define SEC 0x00
#define MIN 0x02
#define HOUR 0x04
#define WEEKDAY 0x06
#define DAYOFMONTH 0x07
#define MONTH 0x08
#define YEAR 0x09
#define CENTURY 0x32



typedef struct {
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t weekday;
    uint8_t day_of_month;
    uint8_t month;
    uint8_t year;
    uint8_t century;
} rtc_info_t;


uint8_t cmos_read(uint8_t reg);
void cmos_get_rtc(rtc_info_t *rtc);