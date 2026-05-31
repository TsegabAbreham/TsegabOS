#ifndef GDT_H
#define GDT_H

#include <stdint.h>


// GDT entry - exactly 8 bytes
typedef struct{
    uint16_t limit_low; // bits 0-15 of segment size
    uint16_t base_low;  // bits 0-15 of base address
    uint8_t base_mid;   // bits 16-23 of base address
    uint8_t access;     // permissions - who can use this and how
    uint8_t granularity; // upper 4-bits of limit + flags
    uint8_t base_high;   // bits 24-31 of base address
} __attribute__((packed)) gdt_entry_t; // Stops compiler from adding padding/spaces in between bits

// What will be loaded to the GDTR register
typedef struct {
    uint16_t size;      //   size of GDT in bytes minus 1
    uint32_t offset;    //   memory address of the GDT array
} __attribute__((packed)) gdtr_t;

void gdt_init(void);

#endif

