#pragma once
#include <stdint.h>
#include "../../kernel/multiboot.h"

#define PAGE_SIZE 4096 // 4KB per page
#define MAX_MEMORY_MB 128 // 128 MB of memory max

#define MAX_PAGES 262144  // temporary safety cap (1GB RAM max)

void pmm_init(uint32_t kernel_end, multiboot_info_t* mbi);
void* pmm_alloc_page();
void pmm_free_page(void* addr);

