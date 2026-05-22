#define COM1 0x3F8

void serial_init();

void serial_write_char(char c);
void serial_write(const char* str);