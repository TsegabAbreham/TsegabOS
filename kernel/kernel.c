#include <stdint.h>  
#include <stddef.h> 
#include <stdarg.h>

#include "../gdt/gdt.h" // GDT table access
#include "../idt/idt.h" // IDT access
#include "../memory/pmm/pmm.h"
#include "../memory/heap/heap.h"
// Drivers
#include "../drivers/keyboard/keyboard.h"
#include "../drivers/mouse/mouse.h"
// GUI 
#include "../GUI/fb/framebuffer.h"
// External helper libraries
#include "../drivers/Serial/libk/kprintf/kprintf.h"

#include "../drivers/Serial/URAT.h"

extern uint32_t kernel_end;


// Framebuffer
static uint32_t* framebuffer;
static uint32_t fb_width;
static uint32_t fb_height;
static uint32_t fb_pitch;

// Memory Mapping
static uint32_t mmaplength;



// Multiboot1 info struct (just the parts we need)
typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
} __attribute__((packed)) multiboot_info_t;

/* -------------------------------------------------------------------------
 * kernel_main — kernel entry point, called from boot.asm
 *
 * This is where your OS starts. At this point:
 *   - We're in 32-bit protected mode
 *   - The stack is set up (by boot.asm)
 *   - Interrupts are disabled
 *   - No standard library exists — we're on our own
 * ------------------------------------------------------------------------- */

void kernel_main(uint32_t magic, multiboot_info_t* mbi) {
    

    framebuffer = (uint32_t*)(uint32_t)mbi->framebuffer_addr;

    fb_width = mbi->framebuffer_width;
    fb_height = mbi->framebuffer_height;
    fb_pitch = mbi->framebuffer_pitch;

    //kprintf("Width: %d\n", (int)fb_width);

    framebuffer_init(
        framebuffer, fb_width, fb_height, fb_pitch
    );

    clear_screen(0x202020);
    mouse_init();
    pmm_init((uint32_t)&kernel_end);
    heap_init(0x01000000, 1024 * 1024);
    serial_init();
    gdt_init();
    idt_init();
    
    mouse_update(fb_width, fb_height);

    while (1) {
        
    }
}