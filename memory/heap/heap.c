#include "heap.h"

static uint32_t heap_start;
static uint32_t heap_end;

static block_header_t* first_block = NULL;



// Initalize the heap
void heap_init(uint32_t start, uint32_t size){
    heap_start = start;
    heap_end = start + size; 

    first_block = (block_header_t*)start;

    first_block->size = size - sizeof(block_header_t);
    first_block->free = 1;
    first_block->next = NULL;
}

// kmalloc

void *kmalloc(size_t size){
    //align to 4 bytes
    size = (size + 3) & ~3;

    block_header_t *curr = first_block;

    while (curr){
        if (curr->free && curr->size >= size){
            curr->free = 0;

            // split block if too large
            if (curr->size > size + sizeof(block_header_t)){
                block_header_t *new_block = 
                    (block_header_t*)((uint8_t*)curr + sizeof(block_header_t) + size);

                new_block->size = curr->size - size - sizeof(block_header_t);
                new_block->free = 1;
                new_block->next = curr->next;

                curr->next = new_block;
                curr->size = size;
            }

            return (void*)(curr + 1);
        }

        curr = curr -> next;
    }

    return NULL;
}

// kfree
void kfree(void *ptr){
    if (!ptr) return;

    block_header_t *block = (block_header_t*)ptr - 1;

    block->free = 1;
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