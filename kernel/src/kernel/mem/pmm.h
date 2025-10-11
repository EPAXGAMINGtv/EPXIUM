#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096 //4kb

void pmm_init(void);
void* pmm_alloc_page(void);
void pmm_free_page(void* ptr);

static inline void set_page_allocated(size_t page_num);
static inline void set_page_free(size_t page_num);
static inline int is_page_allocated(size_t page_num);

extern size_t get_total_physical_memory(void);

#endif