#include "string.h"

void uint_to_str(uint32_t n, char* buf) {
    if (n == 0) { buf[0] = '0'; buf[1] = '\0'; return; }

    char tmp[16];
    int j = 0, i = 0;

    while (n > 0) { tmp[j++] = '0' + (n % 10); n /= 10; }
    while (j > 0)  { buf[i++] = tmp[--j]; }

    buf[i] = '\0';
}