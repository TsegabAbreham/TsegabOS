#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#include "multiboot.h"

#include "../gdt/gdt.h"
#include "../idt/idt.h"

#include "../memory/pmm/pmm.h"
#include "../memory/heap/heap.h"

#include "../GUI/fb/framebuffer.h"
#include "../drivers/mouse/mouse.h"

#include "../drivers/Serial/libk/kprintf/kprintf.h"
#include "../drivers/Serial/URAT.h"

#include "../external/external/lvgl/lvgl.h"

extern uint32_t kernel_end;

// --------------------------------------------------
// FRAMEBUFFER
// --------------------------------------------------
static uint32_t* framebuffer;
static uint32_t fb_width;
static uint32_t fb_height;
static uint32_t fb_pitch;

// --------------------------------------------------
// LVGL BUFFER
// --------------------------------------------------
static uint32_t lvgl_buf[800 * 50];

// --------------------------------------------------
// LVGL FLUSH
// --------------------------------------------------
void lvgl_flush(lv_display_t *disp,
                const lv_area_t *area,
                uint8_t *px_map)
{
    uint32_t *src = (uint32_t*)px_map;

    for(int y = area->y1; y <= area->y2; y++)
    {
        uint32_t *dst =
            framebuffer +
            y * fb_pitch +
            area->x1;

        for(int x = area->x1; x <= area->x2; x++)
        {
            *dst++ = *src++;
        }
    }

    lv_display_flush_ready(disp);
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

    pmm_init((uint32_t)&kernel_end, mbi);
    heap_init();

    kprintf("Kernel starting...\n");

    // --------------------------------------------------
    // FRAMEBUFFER INIT
    // --------------------------------------------------
    framebuffer =
        (uint32_t*)(uint32_t)mbi->framebuffer_addr;

    fb_width  = mbi->framebuffer_width;
    fb_height = mbi->framebuffer_height;

    fb_pitch  = mbi->framebuffer_pitch / 4;

    framebuffer_init(
        framebuffer,
        fb_width,
        fb_height,
        fb_pitch
    );

    clear_screen(0x202020);

    // --------------------------------------------------
    // MOUSE INIT
    // --------------------------------------------------
    mouse_init(fb_width, fb_height);

    // --------------------------------------------------
    // LVGL INIT
    // --------------------------------------------------
    lv_init();

    lv_display_t *disp =
        lv_display_create(fb_width, fb_height);

    lv_display_set_buffers(
        disp,
        lvgl_buf,
        NULL,
        sizeof(lvgl_buf),
        LV_DISPLAY_RENDER_MODE_PARTIAL
    );

    lv_display_set_flush_cb(
        disp,
        lvgl_flush
    );

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
    lv_obj_t *btn =
        lv_btn_create(lv_screen_active());

    lv_obj_set_size(btn, 140, 70);

    lv_obj_align(
        btn,
        LV_ALIGN_CENTER,
        0,
        0
    );

    lv_obj_t *label =
        lv_label_create(btn);

    lv_label_set_text(
        label,
        "HELLO OS"
    );

    lv_obj_center(label);

    kprintf("LVGL UI created\n");

    // --------------------------------------------------
    // MAIN LOOP
    // --------------------------------------------------
    while(1)
    {
        lv_tick_inc(1);

        lv_timer_handler();

        delay();
    }
}