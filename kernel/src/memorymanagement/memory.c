#include "memory.h"

static unsigned char* bitmap = 0;
static size_t total_pages = 0;
static uintptr_t base_address = 0;

#define BIT_SET(bitmap, i)   ((bitmap)[(i)/8] |=  (1 << ((i)%8)))
#define BIT_CLEAR(bitmap, i) ((bitmap)[(i)/8] &= ~(1 << ((i)%8)))
#define BIT_TEST(bitmap, i)  (((bitmap)[(i)/8] >> ((i)%8)) & 1)

void memory_init(uintptr_t base, size_t mem_size) {
    base_address = base;
    total_pages = mem_size / PAGE_SIZE;

    size_t bitmap_size = (total_pages + 7) / 8;

    bitmap = (unsigned char*)base_address;

    size_t bitmap_pages = (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (size_t i = 0; i < bitmap_size; i++) {
        bitmap[i] = 0;
    }

    for (size_t i = 0; i < bitmap_pages; i++) {
        BIT_SET(bitmap, i);
    }

    for (size_t i = bitmap_pages; i < total_pages; i++) {
        BIT_CLEAR(bitmap, i);
    }
}

uintptr_t alloc_page(void) {
    for (size_t i = 0; i < total_pages; i++) {
        if (!BIT_TEST(bitmap, i)) {
            BIT_SET(bitmap, i);
            return base_address + i * PAGE_SIZE;
        }
    }
    return 0; 
}

void free_page(uintptr_t addr) {
    if (addr < base_address) return;

    size_t index = (addr - base_address) / PAGE_SIZE;
    if (index >= total_pages) return;

    BIT_CLEAR(bitmap, index);
}

size_t get_total_pages(void) {
    return total_pages;
}

size_t get_free_pages(void) {
    size_t free = 0;
    for (size_t i = 0; i < total_pages; i++) {
        if (!BIT_TEST(bitmap, i)) free++;
    }
    return free;
}
