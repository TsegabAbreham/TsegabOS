#include <stdint.h>   
#include <stddef.h> 

extern void terminal_print(const char* str);
extern void terminal_backspace(void);
/* -------------------------------------------------------------------------
 * VGA hardware constants
 * ------------------------------------------------------------------------- */
#define VGA_WIDTH       80          /* number of columns on screen */
#define VGA_HEIGHT      25          /* number of rows on screen */
#define VGA_MEMORY      0xB8000     /* physical address of VGA text buffer */

/* -------------------------------------------------------------------------
 * VGA color codes
 * These are the 16 colors available in VGA text mode.
 * Use them to build color attributes for characters.
 * ------------------------------------------------------------------------- */
typedef enum {
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN   = 14,
    VGA_COLOR_WHITE         = 15,
} vga_color;

/* -------------------------------------------------------------------------
 * vga_entry_color — build a color attribute byte
 *
 * The attribute byte packs foreground and background into one byte:
 *   bits 7-4 = background color (0-7 only, high bit is blink)
 *   bits 3-0 = foreground color (0-15)
 *
 * Example: vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK)
 *          = white text on black background
 * ------------------------------------------------------------------------- */
static inline uint8_t vga_entry_color(vga_color fg, vga_color bg) {
    return fg | bg << 4;
}

/* -------------------------------------------------------------------------
 * vga_entry — pack a character + color into a 16-bit VGA entry
 *
 * VGA memory wants a 16-bit value per cell:
 *   high byte = color attribute
 *   low byte  = ASCII character
 * ------------------------------------------------------------------------- */
static inline uint16_t vga_entry(unsigned char c, uint8_t color) {
    return (uint16_t)c | (uint16_t)color << 8;
}

/* -------------------------------------------------------------------------
 * strlen — get length of a null-terminated string
 *
 * We can't use the standard library (there is none), so we write our own.
 * Counts characters until it hits the null terminator '\0'.
 * ------------------------------------------------------------------------- */
static size_t strlen(const char* str) {
    size_t len = 0;
    while (str[len] != '\0')   /* keep going until we hit end of string */
        len++;
    return len;
}

/* -------------------------------------------------------------------------
 * Terminal state
 * We track the cursor position and current color so we know where to write
 * the next character.
 * ------------------------------------------------------------------------- */
static size_t terminal_row;      /* current cursor row (0 = top) */
static size_t terminal_col;      /* current cursor column (0 = left) */
static uint8_t terminal_color;   /* current text color attribute */
static uint16_t* terminal_buf;   /* pointer directly into VGA memory */

/* -------------------------------------------------------------------------
 * terminal_init — set up the terminal
 *
 * Clears the screen by writing blank spaces to every cell, and resets
 * the cursor to the top-left corner.
 * ------------------------------------------------------------------------- */
void terminal_init(void) {
    terminal_row   = 0;
    terminal_col   = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);

    /* Point our buffer pointer at the VGA memory address.
     * We cast the integer address 0xB8000 to a uint16_t pointer so we
     * can write 16-bit VGA entries directly. */
    terminal_buf = (uint16_t*)VGA_MEMORY;

    /* Clear the screen by filling every cell with a blank space */
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            /* Calculate the index: each row has VGA_WIDTH cells */
            const size_t index = y * VGA_WIDTH + x;
            terminal_buf[index] = vga_entry(' ', terminal_color);
        }
    }
}

/* -------------------------------------------------------------------------
 * terminal_putchar — write a single character at the current cursor position
 *
 * Writes the character to VGA memory, then advances the cursor.
 * Handles newlines and wraps to the next line when reaching the right edge.
 * ------------------------------------------------------------------------- */
void terminal_putchar(char c) {
    /* Handle newline — move to start of next row */
    if (c == '\n') {
        terminal_col = 0;
        terminal_row++;
        return;
    }

    /* Write the character to VGA memory at the current cursor position */
    const size_t index = terminal_row * VGA_WIDTH + terminal_col;
    terminal_buf[index] = vga_entry(c, terminal_color);

    /* Advance the cursor — wrap to next line if we hit the right edge */
    if (++terminal_col == VGA_WIDTH) {
        terminal_col = 0;
        terminal_row++;
    }

    /* TODO: handle scrolling when terminal_row >= VGA_HEIGHT */
}

void terminal_backspace(void) {
    if (terminal_col > 0) {
        terminal_col--;
    } else if (terminal_row > 0) {
        // wrap back to previous line
        terminal_row--;
        terminal_col = VGA_WIDTH - 1;
    }
    // erase the character at new cursor position
    const size_t index = terminal_row * VGA_WIDTH + terminal_col;
    terminal_buf[index] = vga_entry(' ', terminal_color);
}

/* -------------------------------------------------------------------------
 * terminal_write — write a string of known length
 * ------------------------------------------------------------------------- */
void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

/* -------------------------------------------------------------------------
 * terminal_print — write a null-terminated string (like puts)
 * ------------------------------------------------------------------------- */
void terminal_print(const char* str) {
    terminal_write(str, strlen(str));
}
