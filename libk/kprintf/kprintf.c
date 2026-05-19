#include "../../drivers/VGA/VGA.h"


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
                terminal_print(s);
            }
            else if (fmt[i] == 'd')
            {
                int val = *(int*)arg;
                arg += sizeof(int);

                char buf[32];
                itoa(val, buf);
                terminal_print(buf);
            }
            else if (fmt[i] == 'x')
            {
                int val = *(int*)arg;
                arg += sizeof(int);

                char buf[32];
                itoa(val, buf); // later replace with hex version
                terminal_print(buf);
            }
        }
        else
        {
            terminal_putchar(fmt[i]);
        }
    }
}