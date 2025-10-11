#ifndef VMM_H
#define VMM_H

#include <stdint.h>

#define PAGE_SIZE 4096
#define PAGE_TABLE_ENTRIES 512

typedef struct {
    uint64_t present : 1;
    uint64_t writable : 1;
    uint64_t user : 1;
    uint64_t write_through : 1;
    uint64_t cache_disable : 1;
    uint64_t accessed : 1;
    uint64_t dirty : 1;
    uint64_t huge_page : 1;
    uint64_t global : 1;
    uint64_t reserved : 3;
    uint64_t address : 40;
    uint64_t reserved2 : 12;
} page_table_entry_t;

void vmm_init(void);
void vmm_map_page(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags);
void vmm_unmap_page(uint64_t virt_addr);
void vmm_set_active_pml4(uint64_t pml4_phys_addr);

#endif