#include "heap.h"
#include "../pmm/pmm.h"

static uint32_t heap_start;
static uint32_t heap_end;

static block_header_t* first_block = NULL;



// Initalize the heap
void heap_init(){
    heap_start = (uint32_t)pmm_alloc_page();
    heap_end = heap_start + PAGE_SIZE; 

    first_block = (block_header_t*)heap_start;

    first_block->size = PAGE_SIZE - sizeof(block_header_t);
    first_block->free = 1;
    first_block->next = NULL;
}

static void heap_expand() {
    block_header_t* new_block =
        (block_header_t*)pmm_alloc_page();

    new_block->size = PAGE_SIZE - sizeof(block_header_t);
    new_block->free = 1;
    new_block->next = NULL;

    block_header_t* curr = first_block;

    while (curr->next) {
        curr = curr->next;
    }

    curr->next = new_block;
}

// kmalloc

void *kmalloc(size_t size)
{
    size = (size + 3) & ~3;

    block_header_t *curr = first_block;

    while (curr) {

        if (curr->free && curr->size >= size) {

            curr->free = 0;

            if (curr->size > size + sizeof(block_header_t)) {

                block_header_t *new_block =
                    (block_header_t*)((uint8_t*)curr +
                    sizeof(block_header_t) + size);

                new_block->size = curr->size - size - sizeof(block_header_t);
                new_block->free = 1;
                new_block->next = curr->next;

                curr->next = new_block;
                curr->size = size;
            }

            return (void*)(curr + 1);
        }

        curr = curr->next;
    }

    // NO SPACE -> grow heap
    heap_expand();

    if (!first_block)
        return NULL;

    // retry once
    return kmalloc(size);
}

// kfree
void kfree(void *ptr){
    if (!ptr) return;

    block_header_t *block = (block_header_t*)ptr - 1;

    block->free = 1;
}

// Memory Copy
void* memcpy(void* dest, const void* src, size_t n) {
    uint8_t* d = dest;
    const uint8_t* s = src;

    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dest;
}



// Debugging tools

size_t heap_get_memory(){
    size_t total = 0;

    block_header_t *curr = first_block;

    while(curr){
        if (curr->free){
            total += curr->size;
        }

        curr = curr->next;
    }

    return total;
}