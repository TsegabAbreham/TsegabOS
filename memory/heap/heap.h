#pragma once
#include <stdint.h>
#include <stddef.h>

void heap_init(uint32_t start, uint32_t size);

typedef struct block_header
{
    size_t size;
    uint8_t free;
    struct block_header* next;
} block_header_t;

void* kmalloc(size_t size);
void kfree(void* ptr);
size_t heap_get_memory();