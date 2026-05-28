#pragma once
#include <stdint.h>
#include <stddef.h>

void heap_init();

typedef struct block_header
{
    size_t size;
    uint8_t free;
    struct block_header* next;
} block_header_t;

void* kmalloc(size_t size);
void kfree(void* ptr);
void* memcpy(void* dest, const void* src, size_t n);
size_t heap_get_memory();
uint32_t heap_get_start();