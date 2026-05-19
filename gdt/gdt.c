// gdt/gdt.c
#include "gdt.h"

// our GDT has 3 entries: null, kernel code, kernel data
#define GDT_ENTRIES 3
static gdt_entry_t gdt[GDT_ENTRIES];
static gdtr_t gdtr;

// helper — fills one GDT entry with the right values
// base  = where the segment starts in memory
// limit = size of the segment
// access, granularity = the flag bytes we described above
static void gdt_set_entry(int i, uint32_t base, uint32_t limit,
                           uint8_t access, uint8_t granularity) {
    // split base address across 3 non-contiguous fields
    gdt[i].base_low  = (base & 0xFFFF);         // bits 0-15
    gdt[i].base_mid  = (base >> 16) & 0xFF;     // bits 16-23
    gdt[i].base_high = (base >> 24) & 0xFF;     // bits 24-31

    // split limit across 2 non-contiguous fields
    gdt[i].limit_low    = (limit & 0xFFFF);      // bits 0-15
    gdt[i].granularity  = (limit >> 16) & 0x0F; // bits 16-19 in lower nibble

    // OR in the flag bits into the upper nibble of granularity
    gdt[i].granularity |= (granularity & 0xF0);

    gdt[i].access = access;
}

// gdt_load is in a separate asm file because lgdt is an assembly instruction
extern void gdt_load(uint32_t gdtr_ptr);

void gdt_init(void) {
    // entry 0: null descriptor — required by x86, must be all zeros
    gdt_set_entry(0, 0, 0, 0, 0);

    // entry 1: kernel code segment
    // base=0, limit=0xFFFFF (4GB with 4KB granularity), access=0x9A, gran=0xCF
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xCF);

    // entry 2: kernel data segment
    // same range, different access byte (0x92 = data, writable)
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xCF);

    // fill the GDTR struct
    gdtr.size   = (sizeof(gdt_entry_t) * GDT_ENTRIES) - 1;
    gdtr.offset = (uint32_t)gdt;  // cast pointer to integer address

    // call assembly to load GDTR and reload segment registers
    gdt_load((uint32_t)&gdtr);
}