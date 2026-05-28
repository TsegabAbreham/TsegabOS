#include <stdint.h>

void kprintf(const char* fmt, ...)
{
    char* arg = (char*)(&fmt + 1);

    for (int i = 0; fmt[i] != '\0'; i++)
    {
        if (fmt[i] == '%')
        {
            i++;

            if (fmt[i] == 's')
            {
                char* s = *(char**)arg;
                arg += sizeof(char*);
                serial_write(s);
            }
            else if (fmt[i] == 'd')
            {
                int val = *(int*)arg;
                arg += sizeof(int);

                char buf[32];
                itoa(val, buf);
                serial_write(buf);
            }
            else if (fmt[i] == 'x')
            {
                uint32_t val = *(uint32_t*)arg;  // unsigned!
                arg += sizeof(uint32_t);

                // convert to hex manually
                char buf[12];
                char hex_chars[] = "0123456789abcdef";
                buf[10] = '\0';
                buf[9]  = ' ';

                for (int j = 8; j >= 1; j--) {
                    buf[j] = hex_chars[val & 0xF];
                    val >>= 4;
                }
                buf[0] = '0';
                buf[1] = 'x';
                buf[9] = '\0';

                serial_write(buf);
            }
        }
        else
        {
            serial_write_char(fmt[i]);
        }
    }
}