#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "multiboot.h"

#include "../gdt/gdt.h"
#include "../idt/idt.h"

#include "../memory/pmm/pmm.h"
#include "../memory/heap/heap.h"
#include "../memory/paging/paging.h"

#include "../GUI/fb/framebuffer.h"
#include "../drivers/mouse/mouse.h"

// GUI Components
#include "../GUI/UI/UI.h"

// Drivers
#include "../drivers/Serial/libk/kprintf/kprintf.h"
#include "../drivers/Serial/URAT.h"

#include "../drivers/ATA/ata.h"

#include "../drivers/pit/pit.h"

// File system
#include "../FS/FAT32/fat32.h"

// External
#include "../external/external/lvgl/lvgl.h"
#include "../GUI/lvgl_manager/init_lvgl.h"


extern uint32_t kernel_end;



// --------------------------------------------------
// FRAMEBUFFER
// --------------------------------------------------
static uint32_t* framebuffer;
static uint32_t fb_width;
static uint32_t fb_height;
static uint32_t fb_pitch;

typedef struct {
    uint32_t addr;
    uint32_t size;
} fb_info_t;

fb_info_t READ_FRAMEBUFFER(multiboot_info_t* mbi) {
    fb_info_t info;

    info.addr = (uint32_t)mbi->framebuffer_addr;
    info.size = mbi->framebuffer_pitch * mbi->framebuffer_height;

    framebuffer = (uint32_t*)info.addr;
    fb_width    = mbi->framebuffer_width;
    fb_height   = mbi->framebuffer_height;
    fb_pitch    = mbi->framebuffer_pitch / 4;

    return info;
}

void INIT_FRAMEBUFFER(multiboot_info_t* mbi) {

    framebuffer_init(
        framebuffer,
        fb_width,
        fb_height,
        fb_pitch
    );

}



// --------------------------------------------------
// LVGL MOUSE INPUT
// --------------------------------------------------
void lvgl_mouse_read(lv_indev_t * indev,
                     lv_indev_data_t * data)
{
    (void)indev;

    data->point.x = mouse_get_x();
    data->point.y = mouse_get_y();

    if(mouse_left_pressed())
        data->state = LV_INDEV_STATE_PRESSED;
    else
        data->state = LV_INDEV_STATE_RELEASED;
}

// --------------------------------------------------
// SIMPLE DELAY
// --------------------------------------------------
void delay()
{
    for(volatile int i = 0; i < 100000; i++);
}

// --------------------------------------------------
// KERNEL MAIN
// --------------------------------------------------
void kernel_main(uint32_t magic,
                 multiboot_info_t* mbi)
{
    (void)magic;

    // --------------------------------------------------
    // SYSTEM INIT
    // --------------------------------------------------
    gdt_init();
    idt_init();
    serial_init();
    pit_init();

    pmm_init((uint32_t)&kernel_end, mbi);
    heap_init();

    fb_info_t fb = READ_FRAMEBUFFER(mbi);

    paging_init((uint32_t)&kernel_end, fb.addr, fb.size);




    // FS test
    // open the file for writing

    fat32_init();
    // create a brand new file
    int create_result = fat32_create_file("HELLO   ", "TXT");
    uint8_t vbuf[512];
    if (create_result == FAT32_OK) {
        kprintf("FAT32: file created successfully\n");

        // now write into it
        fat32_file_t newfile;
        fat32_open("HELLO   ", "TXT", &newfile);

        const uint8_t* msg = (const uint8_t*)"I was created by my own OS!";
        uint32_t written = fat32_write(&newfile, msg, 27);
        fat32_update_dir_entry("HELLO   ", "TXT", written, newfile.first_cluster);

        kprintf("FAT32: wrote %u bytes to new file\n", written);

        // read it back
        fat32_file_t verify;
        fat32_open("HELLO   ", "TXT", &verify);

        uint32_t r = fat32_read(&verify, vbuf, 512);
        vbuf[r] = '\0';
        kprintf("FAT32: verify: %s\n", vbuf);
    }





    kprintf("Kernel starting...\n");

    // --------------------------------------------------
    // FRAMEBUFFER INIT
    // --------------------------------------------------
    INIT_FRAMEBUFFER(mbi);
    clear_screen(0x202020);

    // LVGL INIT
    LVGL_INIT(framebuffer, fb_width, fb_height, fb_pitch);

    // --------------------------------------------------
    // MOUSE INIT
    // --------------------------------------------------
    mouse_init(fb_width, fb_height);

    // --------------------------------------------------
    // INPUT DEVICE
    // --------------------------------------------------
    lv_indev_t *mouse_indev =
        lv_indev_create();

    lv_indev_set_type(
        mouse_indev,
        LV_INDEV_TYPE_POINTER
    );

    lv_indev_set_read_cb(
        mouse_indev,
        lvgl_mouse_read
    );

    // --------------------------------------------------
    // CURSOR
    // --------------------------------------------------
    lv_obj_t *cursor =
        lv_obj_create(lv_screen_active());

    lv_obj_set_size(cursor, 12, 12);

    lv_obj_set_style_radius(
        cursor,
        LV_RADIUS_CIRCLE,
        0
    );

    lv_obj_set_style_bg_color(
        cursor,
        lv_color_hex(0xFFFFFF),
        0
    );

    lv_obj_set_style_border_width(
        cursor,
        0,
        0
    );

    lv_indev_set_cursor(
        mouse_indev,
        cursor
    );

    // --------------------------------------------------
    // BUTTON TEST
    // --------------------------------------------------
    create_button(100, 300, 300, 400, "Test Button");
    create_label(700, 800, (char*)vbuf);
    kprintf("LVGL UI created\n");


    // --------------------------------------------------
    // MAIN LOOP
    // --------------------------------------------------
    while(1)
    {
        lv_tick_inc(1);

        lv_timer_handler();

    }
}