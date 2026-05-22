#include "../../drivers/Serial/URAT.h"


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
                int val = *(int*)arg;
                arg += sizeof(int);

                char buf[32];
                itoa(val, buf); // later replace with hex version
                serial_write(buf);
            }
        }
        else
        {
            serial_write_char(fmt[i]);
        }
    }
}