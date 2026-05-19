#pragma once
#include <stdint.h>

#define PAGE_SIZE 4096 // 4KB per page
#define MAX_MEMORY_MB 128 // 128 MB of memory max

#define MAX_PAGES ((MAX_MEMORY_MB * 1024 * 1024) / PAGE_SIZE)

void pmm_init(uint32_t kernel_end);
void* pmm_alloc_page();
void pmm_free_page(void* addr);

