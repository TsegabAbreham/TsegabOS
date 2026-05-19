#ifndef IDT_H
#define IDT_H

#include <stdint.h>


typedef struct {
    uint16_t offset_low;   // lower 16 bits of handler address
    uint16_t selector;     // GDT selector — always 0x08 (kernel code)
    uint8_t  zero;         // always 0
    uint8_t  type_attr;    // type and flags
    uint16_t offset_high;  // upper 16 bits of handler address
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t size;         // size of IDT in bytes minus 1
    uint32_t offset;       // address of IDT array
} __attribute__((packed)) idtr_t;

void idt_init(void);

#endif

