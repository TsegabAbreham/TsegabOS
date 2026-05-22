#include "keyboard.h"

// outb — write byte to hardware port
// needed to send EOI to PIC after handling IRQ
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}
 
// inb — read byte from hardware port
// used to read scancode from keyboard data port 0x60
static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}


// scancode to ASCII lookup table
// index = scancode from port 0x60
// value = ASCII character (0 = no printable character)



static char scancode_table[128] = {
    0,     // 0x00 — unknown
    0,     // 0x01 — escape
    '1',   // 0x02
    '2',   // 0x03
    '3',   // 0x04
    '4',   // 0x05
    '5',   // 0x06
    '6',   // 0x07
    '7',   // 0x08
    '8',   // 0x09
    '9',   // 0x0A
    '0',   // 0x0B
    '-',   // 0x0C
    '=',   // 0x0D
    0,     // 0x0E — backspace (handle separately later)
    '\t',  // 0x0F — tab
    'q',   // 0x10
    'w',   // 0x11
    'e',   // 0x12
    'r',   // 0x13
    't',   // 0x14
    'y',   // 0x15
    'u',   // 0x16
    'i',   // 0x17
    'o',   // 0x18
    'p',   // 0x19
    '[',   // 0x1A
    ']',   // 0x1B
    '\n',  // 0x1C — enter
    0,     // 0x1D — left ctrl
    'a',   // 0x1E
    's',   // 0x1F
    'd',   // 0x20
    'f',   // 0x21
    'g',   // 0x22
    'h',   // 0x23
    'j',   // 0x24
    'k',   // 0x25
    'l',   // 0x26
    ';',   // 0x27
    '\'',  // 0x28
    '`',   // 0x29
    0,     // 0x2A — left shift
    '\\',  // 0x2B
    'z',   // 0x2C
    'x',   // 0x2D
    'c',   // 0x2E
    'v',   // 0x2F
    'b',   // 0x30
    'n',   // 0x31
    'm',   // 0x32
    ',',   // 0x33
    '.',   // 0x34
    '/',   // 0x35
    0,     // 0x36 — right shift
    '*',   // 0x37 — keypad *
    0,     // 0x38 — left alt
    ' ',   // 0x39 — space
    0,     // 0x3A — caps lock
    0,     // 0x3B — F1
    0,     // 0x3C — F2
    0,     // 0x3D — F3
    0,     // 0x3E — F4
    0,     // 0x3F — F5
    0,     // 0x40 — F6
    0,     // 0x41 — F7
    0,     // 0x42 — F8
    0,     // 0x43 — F9
    0,     // 0x44 — F10
    0,     // 0x45 — num lock
    0,     // 0x46 — scroll lock
    '7',   // 0x47 — keypad 7
    '8',   // 0x48 — keypad 8
    '9',   // 0x49 — keypad 9
    '-',   // 0x4A — keypad -
    '4',   // 0x4B — keypad 4
    '5',   // 0x4C — keypad 5
    '6',   // 0x4D — keypad 6
    '+',   // 0x4E — keypad +
    '1',   // 0x4F — keypad 1
    '2',   // 0x50 — keypad 2
    '3',   // 0x51 — keypad 3
    '0',   // 0x52 — keypad 0
    '.',   // 0x53 — keypad .
    0, 0, 0,  // 0x54-0x56 — unused
    0,     // 0x57 — F11
    0,     // 0x58 — F12
    // rest are 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0x59-0x68
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 0x69-0x78
    0, 0, 0, 0, 0, 0, 0                               // 0x79-0x7F
};

void keyboard_init(void){

}

static uint8_t shift_pressed = 0;

void keyboard_handler(void) {
    uint8_t scancode = inb(0x60);

    // detect shift press and release
    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        outb(0x20, 0x20); // Tells that it is done with the interrupt (EOI)
        return;
    }
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        outb(0x20, 0x20);
        return;
    }
    
    // Backspace
    if (scancode == 0x0E){
        /*
        Find another way of outputting
        terminal_backspace();
        */
        outb(0x20, 0x20);
        return;    
    }

    // key release — ignore
    if (scancode & 0x80) {
        outb(0x20, 0x20);
        return;
    }

    // key press — look up character
    char c = scancode_table[scancode];
    if (c != 0) {
        // if shift held, convert to uppercase
        if (shift_pressed && c >= 'a' && c <= 'z') {
            c = c - 32;   // ASCII lowercase to uppercase
        }
        if (shift_pressed && c >= '1' && c <= '9') {
            c = c - 32;   // ASCII lowercase to uppercase
        }
        /*
        Find another way of using the keyboard driver since VGA is not supported anymore
        terminal_putchar(c);
        */

        }

    

    outb(0x20, 0x20);
}