#include <stdint.h>

#include "../../drivers/Serial/libk/kprintf/kprintf.h"
#include "../pmm/pmm.h"
#include "../../drivers/Serial/libk/tools/tools.h"

extern void paging_load_directory(uint32_t phys_addr);
extern void paging_enable(void);


/* ------------------- Paging Logic ----------------------------
        Page Directory          Page Table            Physical RAM
        ┌───────────┐          ┌───────────┐
        │ entry 0   │─────────►│ entry 0   │─────────► 0x00001000
        │ entry 1   │    ┌────►│ entry 1   │─────────► 0x00002000
 CR3───►│ entry 2   │────┘     │  ...      │
        │  ...      │          │ entry 1023│
        │ entry 1023│          └───────────┘
        └───────────┘
*/

// ==============================================
// =============== Paging Code ==================
// ==============================================

// Helper Macros
/*
 When you give a 32-bit virtual address ex: 0x00401000
 it needs to get split into 3 parts the page directory, page table, and page offset
 Page Directory (the first 10 bits or 31-22) -> finds which section on memory
 Page Table (the middle 10 bits  or 22-12) -> finds the page within that section
 Page Offset (the last 12 bit 12-0) -> finds the exact byte within that page, no lookup at this stage
*/
#define PD_INDEX(va)    ((((va) >> 22)) & 0x3FF)
#define PT_INDEX(va)    (((va) >> 12) & 0x3FF)
#define PG_OFFSET(va)   ((va) & 0xFFF)

// Flags for the last 12 bits
#define PAGE_PRESENT  (1 << 0)   // bit 0
#define PAGE_WRITABLE (1 << 1)   // bit 1
#define PAGE_USER     (1 << 2)   // bit 2



// Page Table
typedef struct {
    uint32_t entries[1024];
} __attribute__((aligned(4096))) page_table_t;

// Page Directory
typedef struct {
    uint32_t entries[1024];
    page_table_t *tables[1024];
} __attribute__((aligned(4096))) page_directory_t;


void map_page(page_directory_t *dir, uint32_t virtual_address, uint32_t physical_address, uint32_t flags) {
    // Mapping step 1: Take the virtual address and find which Page Directory entry it belongs to
    uint32_t pd_indx = PD_INDEX(virtual_address); // Page Directory entry
    uint32_t pt_indx = PT_INDEX(virtual_address); // Page Table entry

    // Mapping step 2: If that Page Directory entry has no Page Table yet — create one
    // checks the last bit of page tabel entry (32-bits) flag -> 1 present, 0 not present
    // If 0 use kmalloc to allocate space for the page table
    if (!(dir->entries[pd_indx] & PAGE_PRESENT)) {
        page_table_t *table = (page_table_t *)pmm_alloc_page();
        memset(table, 0, sizeof(page_table_t)); // Zero it out to avoid garbage entries

        // Store it in both arrays
        dir -> tables[pd_indx] = table;
        // Enables the present and writable flags for the CPU's copy
        dir -> entries[pd_indx] = (uint32_t)table | PAGE_PRESENT | PAGE_WRITABLE;
    }

    // Mapping step 3: Take the virtual address and find which Page Table entry it belongs to
    page_table_t *table = dir->tables[pd_indx];

    // Mapping step 4: Write the physical address + flags into that Page Table entry
    table -> entries[pt_indx] = (physical_address & 0xFFFFF000) | flags | PAGE_PRESENT;

}

// Initalize paging

page_directory_t *kernel_dir = NULL;

void paging_init(uint32_t kernel_end, uint32_t fb_addr, uint32_t fb_size) {
    // Create the page directory
    kernel_dir = (page_directory_t *)pmm_alloc_page();
    pmm_alloc_page(); // allocate second page for the tables[] portion
    memset(kernel_dir, 0, sizeof(page_directory_t));

    // Identity Map 0 to 64MB
    uint32_t addr = 0;
    while (addr < 0x4000000) {
        map_page(kernel_dir, addr, addr, PAGE_WRITABLE); // Map virtual -> physical
        addr += 0x1000; // Move on to the next page
    }

    // Map the framebuffer since it is above the 64MB mapping range (0xD000000)
    uint32_t fb_start = fb_addr & 0xFFFFF000;
    uint32_t fb_end   = fb_start + fb_size + 0x1000;
    for (uint32_t a = fb_start; a < fb_end; a += 0x1000) {
        map_page(kernel_dir, a, a, PAGE_WRITABLE);
    }


    paging_load_directory((uint32_t)kernel_dir);

    paging_enable();


}
