#include <stdint.h>  
#include <stddef.h> 
#include <stdarg.h>

#include "multiboot.h"

#include "../gdt/gdt.h" // GDT table access
#include "../idt/idt.h" // IDT access

// Memory 
#include "../memory/pmm/pmm.h"
#include "../memory/heap/heap.h"

// Drivers
#include "../drivers/keyboard/keyboard.h"
#include "../drivers/mouse/mouse.h"
// GUI 
#include "../GUI/fb/framebuffer.h"
#include "../GUI/bitmap/bitmap.h"

// External helper libraries
#include "../drivers/Serial/libk/kprintf/kprintf.h"

#include "../drivers/Serial/URAT.h"

extern uint32_t kernel_end;





// Framebuffer
static uint32_t* framebuffer;
static uint32_t fb_width;
static uint32_t fb_height;
static uint32_t fb_pitch;


void kernel_main(uint32_t magic, multiboot_info_t* mbi) {
    
    gdt_init();
    idt_init();
    serial_init();

    pmm_init((uint32_t)&kernel_end, mbi);
    heap_init();


    int* a = (int*)kmalloc(16);

    a[0] = 0x11111111;
    a[1] = 0x22222222;
    a[2] = 0x33333333;

    kprintf("%x %x %x\n", a[0], a[1], a[2]);


    framebuffer = (uint32_t*)(uint32_t)mbi->framebuffer_addr;

    fb_width = mbi->framebuffer_width;
    fb_height = mbi->framebuffer_height;
    fb_pitch = mbi->framebuffer_pitch;

    //kprintf("Width: %d\n", (int)fb_width);

    framebuffer_init(
        framebuffer, fb_width, fb_height, fb_pitch
    );


    clear_screen(0x202020);
    draw_rect(500, 500, 100, 100, 0xFFFFFF);
    draw_rect(495, 495, 100, 100, 0x999999);

    mouse_init(fb_width, fb_height);

    

    while (1) {
        mouse_render();
    }
}