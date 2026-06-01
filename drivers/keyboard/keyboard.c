#include "keyboard.h"

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static char scancode_table[128] = {
    0,      // 0x00
    0,      // 0x01 — escape
    '1','2','3','4','5','6','7','8','9','0','-','=',
    0,      // 0x0E — backspace
    '\t',   // 0x0F
    'q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0,      // 0x1D — left ctrl
    'a','s','d','f','g','h','j','k','l',';','\'','`',
    0,      // 0x2A — left shift
    '\\','z','x','c','v','b','n','m',',','.','/',
    0,      // 0x36 — right shift
    '*',
    0,      // 0x38 — left alt
    ' ',    // 0x39
    0,0,0,0,0,0,0,0,0,0,  // 0x3A-0x43 caps/F1-F9
    0,0,0,                 // 0x44-0x46 F10/numlock/scroll
    '7','8','9','-','4','5','6','+','1','2','3','0','.',
    0,0,0,0,0,             // 0x54-0x58
    // 0x59-0x7F
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0
};

// Shifted versions of scancode_table entries
static char scancode_table_shifted[128] = {
    0,0,
    '!','@','#','$','%','^','&','*','(',')','_','+',
    0,'\t',
    'Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
    0,
    'A','S','D','F','G','H','J','K','L',':','"','~',
    0,
    '|','Z','X','C','V','B','N','M','<','>','?',
    0,'*',0,' ',
    // rest zeros
    0,0,0,0,0,0,0,0,0,0,
    0,0,0,
    '7','8','9','-','4','5','6','+','1','2','3','0','.',
    0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0
};

static uint8_t shift_pressed = 0;

// Ring buffer for keyboard input
#define KB_BUF_SIZE 16
static char kb_buf[KB_BUF_SIZE];
static uint8_t kb_head = 0;  // write index
static uint8_t kb_tail = 0;  // read index

static void kb_buf_push(char c) {
    uint8_t next = (kb_head + 1) % KB_BUF_SIZE;
    if (next != kb_tail) {   // drop if full
        kb_buf[kb_head] = c;
        kb_head = next;
    }
}

// Returns 0 if buffer empty, otherwise pops and returns the character
char kb_buf_pop(void) {
    if (kb_head == kb_tail) return 0;
    char c = kb_buf[kb_tail];
    kb_tail = (kb_tail + 1) % KB_BUF_SIZE;
    return c;
}

void keyboard_init(void) {
    kb_head = 0;
    kb_tail = 0;
}

// Called from your IRQ1 handler — reads one scancode and buffers it
void keyboard_handler(void) {
    uint8_t scancode = inb(0x60);

    // Shift press
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        outb(0x20, 0x20);
        return;
    }
    // Shift release
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        outb(0x20, 0x20);
        return;
    }
    // Backspace press
    if (scancode == 0x0E) {
        kb_buf_push('\b');
        outb(0x20, 0x20);
        return;
    }
    // Ignore all key-release events (bit 7 set)
    if (scancode & 0x80) {
        outb(0x20, 0x20);
        return;
    }

    // Key press — look up character
    char c = shift_pressed
        ? scancode_table_shifted[scancode]
        : scancode_table[scancode];

    if (c != 0) {
        kb_buf_push(c);
    }

    outb(0x20, 0x20);  // EOI always sent
}