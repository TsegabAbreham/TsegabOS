#include <stdint.h>  
#include <stddef.h> 
#include <stdarg.h>

#include "../drivers/VGA/VGA.h"
#include "../gdt/gdt.h" // GDT table access
#include "../idt/idt.h" // IDT access
#include "../memory/pmm/pmm.h"
#include "../memory/heap/heap.h"
// Drivers
#include "../drivers/keyboard/keyboard.h"

// External helper libraries
#include "../libk/kprintf/kprintf.h"




extern uint32_t kernel_end;


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
void kernel_main(void) {
    
    // Initalize things
    pmm_init((uint32_t)&kernel_end);
    heap_init(0x01000000, 1024 * 1024);
    gdt_init();
    terminal_init();
    idt_init();
    keyboard_init();

    // Main
    kprintf("======== HEAP TEST ========\n");

    int *a = kmalloc(4); // Allocate 4 bytes at the address pointed

    *a = 111; // Write data onto the allocated address


    kprintf("a=%x\n", a);

    kprintf("write test passed\n");

    kfree(a);

    int *d = kmalloc(4);
    kprintf("d=%x\n", d);
    kprintf("MEMORY FREEING TEST PASSED!\n");

    kprintf("=== SPLIT TEST ===\n");

    int* e = kmalloc(32);
    int* f = kmalloc(8);

    kprintf("a=%x b=%x\n", e, f);

    kprintf("===== TOTAL SPACE LEFT =====\n");
    kprintf("SIZE = %d", heap_get_memory());

    while(1) {}
}
