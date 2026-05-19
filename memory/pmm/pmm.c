#include "pmm.h"

static uint8_t page_bitmap[MAX_PAGES / 8];


// -------- HELPERS ----------


/* Bit operations allows bit manipulation on the
 bitmap to mark a bit as used(1) and unused (0) accorss pages. */

static inline void set_bit(uint32_t bit) // Marks as used 
{
    page_bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void clear_bit(uint32_t bit) // Marked as free or clears the used (basically reversing set_bit)
{
    page_bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static inline int test_bit(uint32_t bit)
{
    return page_bitmap[bit / 8] & (1 << (bit % 8));
}

// Address Conversion 
static inline uint32_t addr_to_page(uint32_t addr)
{
    return addr / PAGE_SIZE;
}

static inline uint32_t page_to_addr (uint32_t page){
    return page * PAGE_SIZE;
}


// Others
static int find_free_page(){
    for (uint32_t i = 0; i < MAX_PAGES; i++){
        if (!test_bit(i)){
            return i; // Page found return page num
        }
    }
    return -1; // No free page left
}


// Initalize PMM
void pmm_init(uint32_t kernel_end){
    // Mark everything as free space at beginning
    
    /* However this line is FLAWED meaning it doesn't consider spaces for VGA memory or low memory (BIOS AREA) 
    assumes everything is free at boot*/
    for (uint32_t i = 0; i < MAX_PAGES; i++){
        clear_bit(i);
    }

    // Reserve some for kernel memory
    uint32_t end_page = addr_to_page(kernel_end);

    for (uint32_t i =0; i < end_page; i++){
        set_bit(i);
    }

}

// PMM allocate page
void *pmm_alloc_page(){
    int page = find_free_page();

    if (page == -1){
        return 0; // No memory left
    }

    set_bit(page);

    return (void*)(page * PAGE_SIZE);
}

// PMM free page

void pmm_free_page(void *addr){
    uint32_t page = (uint32_t)addr / PAGE_SIZE;

    clear_bit(page);
}