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
