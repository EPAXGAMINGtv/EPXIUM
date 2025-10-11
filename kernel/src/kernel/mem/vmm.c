#include "vmm.h"
#include "pmm.h"
#include <limine.h>
#include <stdint.h>

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

static page_table_entry_t* pml4 = NULL;

void vmm_init(void) {
    uint64_t hhdm_offset = hhdm_request.response->offset;
    pml4 = (page_table_entry_t*)(hhdm_offset + (uint64_t)pmm_alloc_page());
    for (size_t i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        pml4[i] = (page_table_entry_t){0};
    }

    uint64_t kernel_virt_base = 0xFFFF800000000000;
    for (size_t i = 0; i < 1 * 1024 * 1024; i += PAGE_SIZE) {
        vmm_map_page(kernel_virt_base + i, i, 0x3);
    }

    uint64_t hhdm_virt_base = hhdm_offset;
    for (size_t i = 0; i < get_total_physical_memory(); i += PAGE_SIZE) {
        vmm_map_page(hhdm_virt_base + i, i, 0x3);
    }

    vmm_set_active_pml4((uint64_t)pml4 - hhdm_offset);
}

void vmm_map_page(uint64_t virt_addr, uint64_t phys_addr, uint64_t flags) {
    uint64_t hhdm_offset = hhdm_request.response->offset;
    uint64_t pml4_index = (virt_addr >> 39) & 0x1FF;
    uint64_t pdpt_index = (virt_addr >> 30) & 0x1FF;
    uint64_t pd_index = (virt_addr >> 21) & 0x1FF;
    uint64_t pt_index = (virt_addr >> 12) & 0x1FF;

    if (!pml4[pml4_index].present) {
        page_table_entry_t* pdpt = (page_table_entry_t*)(hhdm_offset + (uint64_t)pmm_alloc_page());
        for (size_t i = 0; i < PAGE_TABLE_ENTRIES; i++) {
            pdpt[i] = (page_table_entry_t){0};
        }
        pml4[pml4_index] = (page_table_entry_t){
            .present = 1,
            .writable = 1,
            .user = 0,
            .address = ((uint64_t)pdpt - hhdm_offset) >> 12
        };
    }

    page_table_entry_t* pdpt = (page_table_entry_t*)(hhdm_offset + (pml4[pml4_index].address << 12));
    if (!pdpt[pdpt_index].present) {
        page_table_entry_t* pd = (page_table_entry_t*)(hhdm_offset + (uint64_t)pmm_alloc_page());
        for (size_t i = 0; i < PAGE_TABLE_ENTRIES; i++) {
            pd[i] = (page_table_entry_t){0};
        }
        pdpt[pdpt_index] = (page_table_entry_t){
            .present = 1,
            .writable = 1,
            .user = 0,
            .address = ((uint64_t)pd - hhdm_offset) >> 12
        };
    }

    page_table_entry_t* pd = (page_table_entry_t*)(hhdm_offset + (pdpt[pdpt_index].address << 12));
    if (!pd[pd_index].present) {
        page_table_entry_t* pt = (page_table_entry_t*)(hhdm_offset + (uint64_t)pmm_alloc_page());
        for (size_t i = 0; i < PAGE_TABLE_ENTRIES; i++) {
            pt[i] = (page_table_entry_t){0};
        }
        pd[pd_index] = (page_table_entry_t){
            .present = 1,
            .writable = 1,
            .user = 0,
            .address = ((uint64_t)pt - hhdm_offset) >> 12
        };
    }

    page_table_entry_t* pt = (page_table_entry_t*)(hhdm_offset + (pd[pd_index].address << 12));
    pt[pt_index] = (page_table_entry_t){
        .present = (flags & 0x1) ? 1 : 0,
        .writable = (flags & 0x2) ? 1 : 0,
        .user = (flags & 0x4) ? 1 : 0,
        .address = phys_addr >> 12
    };
}

void vmm_unmap_page(uint64_t virt_addr) {
    uint64_t hhdm_offset = hhdm_request.response->offset;
    uint64_t pml4_index = (virt_addr >> 39) & 0x1FF;
    uint64_t pdpt_index = (virt_addr >> 30) & 0x1FF;
    uint64_t pd_index = (virt_addr >> 21) & 0x1FF;
    uint64_t pt_index = (virt_addr >> 12) & 0x1FF;

    if (!pml4[pml4_index].present) return;
    page_table_entry_t* pdpt = (page_table_entry_t*)(hhdm_offset + (pml4[pml4_index].address << 12));
    if (!pdpt[pdpt_index].present) return;
    page_table_entry_t* pd = (page_table_entry_t*)(hhdm_offset + (pdpt[pdpt_index].address << 12));
    if (!pd[pd_index].present) return;
    page_table_entry_t* pt = (page_table_entry_t*)(hhdm_offset + (pd[pd_index].address << 12));

    pt[pt_index] = (page_table_entry_t){0};
    asm volatile("invlpg (%0)" : : "r"(virt_addr) : "memory");
}

void vmm_set_active_pml4(uint64_t pml4_phys_addr) {
    asm volatile("mov %0, %%cr3" : : "r"(pml4_phys_addr) : "memory");
}