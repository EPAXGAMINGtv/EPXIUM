#include "pmm.h"
#include <limine.h>
#include <stdint.h>

static size_t total_pages = 0;
static uint8_t *memory_bitmap = NULL;
static size_t bitmap_size = 0;

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

static inline size_t get_page_frame_number(void* ptr) {
    return (size_t)ptr / PAGE_SIZE;
}

static inline size_t get_total_pages(void) {
    return total_pages;
}

void pmm_init(void) {
    struct limine_memmap_response* memmap = memmap_request.response;
    if (!memmap) {
        for(;;);
    }

    size_t total_memory = 0;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            total_memory += entry->length;
        }
    }

    total_pages = total_memory / PAGE_SIZE;
    if (total_memory % PAGE_SIZE != 0) {
        total_pages++;
    }

    bitmap_size = (total_pages + 7) / 8;

    struct limine_memmap_entry* bitmap_entry = NULL;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= bitmap_size) {
            bitmap_entry = entry;
            break;
        }
    }

    if (!bitmap_entry) {
        for(;;);
    }

    memory_bitmap = (uint8_t*)bitmap_entry->base;
    bitmap_entry->base += bitmap_size;
    bitmap_entry->length -= bitmap_size;

    for (size_t i = 0; i < bitmap_size; i++) {
        memory_bitmap[i] = 0xFF;
    }

    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            size_t start_page = entry->base / PAGE_SIZE;
            size_t page_count = entry->length / PAGE_SIZE;
            if (entry->length % PAGE_SIZE != 0) {
                page_count++;
            }

            for (size_t page = start_page; page < start_page + page_count; page++) {
                set_page_free(page);
            }
        }
    }

    size_t bitmap_start_page = get_page_frame_number(memory_bitmap);
    size_t bitmap_page_count = (bitmap_size + PAGE_SIZE - 1) / PAGE_SIZE;
    for (size_t page = bitmap_start_page; page < bitmap_start_page + bitmap_page_count; page++) {
        set_page_allocated(page);
    }
}

void* pmm_alloc_page(void) {
    for (size_t page_num = 0; page_num < total_pages; page_num++) {
        if (!is_page_allocated(page_num)) {
            set_page_allocated(page_num);
            return (void*)(page_num * PAGE_SIZE);
        }
    }
    return NULL;
}

void pmm_free_page(void* ptr) {
    size_t page_num = get_page_frame_number(ptr);
    if (page_num < total_pages) {
        set_page_free(page_num);
    }
}

static inline void set_page_allocated(size_t page_num) {
    if (page_num >= total_pages) return;
    size_t byte_index = page_num / 8;
    size_t bit_index = page_num % 8;
    memory_bitmap[byte_index] |= (1 << bit_index);
}

static inline void set_page_free(size_t page_num) {
    if (page_num >= total_pages) return;
    size_t byte_index = page_num / 8;
    size_t bit_index = page_num % 8;
    memory_bitmap[byte_index] &= ~(1 << bit_index);
}

static inline int is_page_allocated(size_t page_num) {
    if (page_num >= total_pages) return 0;
    size_t byte_index = page_num / 8;
    size_t bit_index = page_num % 8;
    return (memory_bitmap[byte_index] & (1 << bit_index)) != 0;
}

static inline void set_page_mapped(size_t page_num) {
    set_page_allocated(page_num);
}

static inline void set_page_unmapped(size_t page_num) {
    set_page_free(page_num);
}

static inline int is_page_mapped(size_t page_num) {
    return is_page_allocated(page_num);
}

size_t get_total_physical_memory(void) {
    struct limine_memmap_response* memmap = memmap_request.response;
    if (!memmap) {
        return 0;
    }

    size_t total_memory = 0;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            total_memory += entry->length;
        }
    }
    return total_memory;
}