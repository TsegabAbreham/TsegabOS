#pragma once
#include <stdint.h>

// Flags
#define PAGE_PRESENT   (1 << 0)
#define PAGE_WRITABLE  (1 << 1)
#define PAGE_USER      (1 << 2)

// Macros
#define PD_INDEX(va)   (((va) >> 22) & 0x3FF)
#define PT_INDEX(va)   (((va) >> 12) & 0x3FF)
#define PG_OFFSET(va)  ((va) & 0xFFF)

// Structs
typedef struct {
    uint32_t entries[1024];
} __attribute__((aligned(4096))) page_table_t;

typedef struct {
    uint32_t      entries[1024];
    page_table_t *tables[1024];
} __attribute__((aligned(4096))) page_directory_t;

// Global kernel page directory
extern page_directory_t *kernel_dir;

// Functions
void paging_init(uint32_t kernel_end, uint32_t fb_addr, uint32_t fb_size);
void map_page(page_directory_t *dir, uint32_t virtual_address, uint32_t physical_address, uint32_t flags);