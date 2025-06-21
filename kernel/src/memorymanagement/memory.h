#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

void memory_init(uintptr_t base_addr, size_t mem_size);
uintptr_t alloc_page(void);
void free_page(uintptr_t addr);
size_t get_total_pages(void);
size_t get_free_pages(void);

#endif
