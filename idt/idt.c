#include "idt.h"
#include "../drivers/keyboard/keyboard.h"

#define IDT_ARRAY 256
static idt_entry_t idt[IDT_ARRAY];
static idtr_t idtr;


// declare assembly stubs as external functions
extern void isr0_stub(void);
extern void isr13_stub(void);
extern void isr14_stub(void);
extern void irq0_stub(void);
extern void irq1_stub(void);

extern void idt_load(uint32_t);
extern void terminal_print(const char* str);
extern void keyboard_handler(void);

// port I/O helper functions — x86 specific instructions
static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

void idt_set_entry(int n, uint32_t handler, uint16_t selector, uint8_t type_attr) {
    idt[n].offset_low  = handler & 0xFFFF;         // lower 16 bits of address
    idt[n].offset_high = (handler >> 16) & 0xFFFF; // upper 16 bits of address
    idt[n].selector    = selector;                  // always 0x08 (kernel code)
    idt[n].zero        = 0;                         // always 0
    idt[n].type_attr   = type_attr;                 // always 0x8E for us
}

void pic_remap(void) {
    // tell both PICs we're reinitializing
    outb(0x20, 0x11);   // master PIC command port
    outb(0xA0, 0x11);   // slave PIC command port

    // set new interrupt vector offsets
    outb(0x21, 0x20);   // master PIC: IRQs start at interrupt 32 (0x20)
    outb(0xA1, 0x28);   // slave PIC: IRQs start at interrupt 40 (0x28)

    // set up master/slave relationship
    outb(0x21, 0x04);   // master: slave is on IRQ2
    outb(0xA1, 0x02);   // slave: its cascade identity

    // set 8086 mode
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // unmask all interrupts
    outb(0x21, 0x00);
    outb(0xA1, 0x00);
}

void idt_init(void) {
    pic_remap();                    // remap PIC first
    inb(0x60); 
    // fill IDT entries pointing at your stubs
    idt_set_entry(0,  (uint32_t)isr0_stub,  0x08, 0x8E);  // divide by zero
    idt_set_entry(13, (uint32_t)isr13_stub, 0x08, 0x8E);  // GPF
    idt_set_entry(14, (uint32_t)isr14_stub, 0x08, 0x8E);  // page fault
    idt_set_entry(32, (uint32_t)irq0_stub, 0x08, 0x8E);  // timer
    idt_set_entry(33, (uint32_t)irq1_stub,  0x08, 0x8E);  // keyboard
    // load the IDT
    idtr.size   = (sizeof(idt_entry_t) * 256) - 1;
    idtr.offset = (uint32_t)idt;
    idt_load((uint32_t)&idtr);
    // enable interrupts — CPU ignores interrupts until you do this
    __asm__ volatile ("sti");
}

void isr_handler(uint32_t int_number) {
    if (int_number == 32) {
        outb(0x20, 0x20);  // send EOI, move on
        return;
    }
    if (int_number == 33) {
        keyboard_handler();
        return;
    }
    char msg[] = "INT: xx\n";
    msg[5] = '0' + ((int_number / 10) % 10);
    msg[6] = '0' + (int_number % 10);
    terminal_print(msg);
    terminal_print("GPF hit\n");
    while(1){}
}