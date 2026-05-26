#include <stdint.h>

void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = dest;
    const uint8_t* s = src;

    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dest;
}

void* memset(void* dest, int c, size_t n)
{
    uint8_t* d = dest;

    for (size_t i = 0; i < n; i++) {
        d[i] = (uint8_t)c;
    }

    return dest;
}

int memcmp(const void* a, const void* b, size_t n)
{
    const uint8_t* x = a;
    const uint8_t* y = b;

    for (size_t i = 0; i < n; i++) {
        if (x[i] != y[i])
            return x[i] - y[i];
    }

    return 0;
}

size_t strlen(const char* s)
{
    size_t len = 0;

    while (s[len] != '\0') {
        len++;
    }

    return len;
}

char* strcpy(char* dest, const char* src)
{
    char* out = dest;

    while (*src) {
        *dest++ = *src++;
    }

    *dest = '\0';

    return out;
}

int strcmp(const char* a, const char* b)
{
    while (*a && (*a == *b)) {
        a++;
        b++;
    }

    return *(unsigned char*)a - *(unsigned char*)b;
}