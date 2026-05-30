#include "idt.h"

#include "../drivers/Serial/libk/kprintf/kprintf.h"
#include "../drivers/keyboard/keyboard.h"
#include "../drivers/pit/pit.h"
#include "../drivers/mouse/mouse.h"

#define IDT_ARRAY 256

static idt_entry_t idt[IDT_ARRAY];
static idtr_t idtr;

// ISR stubs
extern void isr0_stub(void);
extern void isr13_stub(void);
extern void isr14_stub(void);

// IRQ stubs
extern void irq0_stub(void);
extern void irq1_stub(void);
extern void irq12_stub(void);

extern void idt_load(uint32_t);

// I/O
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void idt_set_entry(
    int n,
    uint32_t handler,
    uint16_t selector,
    uint8_t type_attr
) {
    idt[n].offset_low  = handler & 0xFFFF;
    idt[n].offset_high = (handler >> 16) & 0xFFFF;

    idt[n].selector  = selector;
    idt[n].zero      = 0;
    idt[n].type_attr = type_attr;
}

void pic_remap(void) {
    uint8_t master_mask = inb(0x21);
    uint8_t slave_mask  = inb(0xA1);

    // starts PIC init
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // vector offsets
    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    // tell master/slave wiring
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // 8086 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // restore masks
    outb(0x21, master_mask);
    outb(0xA1, slave_mask);
}

void irq_set_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = 0x21;
    } else {
        port = 0xA1;
        irq -= 8;
    }

    value = inb(port) | (1 << irq);
    outb(port, value);
}

void irq_clear_mask(uint8_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = 0x21;
    } else {
        port = 0xA1;
        irq -= 8;
    }

    value = inb(port) & ~(1 << irq);
    outb(port, value);
}

void idt_init(void) {

    // clear IDT
    for (int i = 0; i < IDT_ARRAY; i++) {
        idt_set_entry(i, 0, 0, 0);
    }

    // exceptions
    idt_set_entry(0,  (uint32_t)isr0_stub,  0x08, 0x8E);
    idt_set_entry(13, (uint32_t)isr13_stub, 0x08, 0x8E);
    idt_set_entry(14, (uint32_t)isr14_stub, 0x08, 0x8E);

    // remap PIC
    pic_remap();

    // IRQs
    idt_set_entry(32, (uint32_t)irq0_stub,  0x08, 0x8E); // timer
    idt_set_entry(33, (uint32_t)irq1_stub,  0x08, 0x8E); // keyboard
    idt_set_entry(44, (uint32_t)irq12_stub, 0x08, 0x8E); // mouse

    // load IDT
    idtr.size   = sizeof(idt_entry_t) * IDT_ARRAY - 1;
    idtr.offset = (uint32_t)idt;

    idt_load((uint32_t)&idtr);

    // mask everything first
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);

    // enable:
    // IRQ0 timer
    // IRQ1 keyboard
    // IRQ2 cascade
    // IRQ12 mouse

    irq_clear_mask(0);
    irq_clear_mask(1);
    irq_clear_mask(2);
    irq_clear_mask(12);

    __asm__ volatile ("sti");
}

void isr_handler(uint32_t int_number) {

    // =============================
    // ========= ISRs ==============
    // =============================
    // PAGE FAULT
    if (int_number == 14) {
        uint32_t fault_address;
        asm volatile ("mov %%cr2, %0" : "=r"(fault_address));
        kprintf("PAGE FAULT!");
        kprintf("fault_address is %x\n", fault_address);
        for (;;);
    }

    // ==============================
    // ========== IRQs ==============
    // ==============================
    // TIMER
    if (int_number == 32) {
        pit_handler();

        outb(0x20, 0x20);
        return;
    }

    // KEYBOARD
    if (int_number == 33) {
        keyboard_handler();

        outb(0x20, 0x20);
        return;
    }


    // MOUSE
    if (int_number == 44) {
        mouse_handler();

        outb(0xA0, 0x20);
        outb(0x20, 0x20);

        return;
    }

    char msg[] = "INT: xx\n";

    msg[5] = '0' + ((int_number / 10) % 10);
    msg[6] = '0' + (int_number % 10);

    kprintf("%s", msg);

    while (1) {}
}
