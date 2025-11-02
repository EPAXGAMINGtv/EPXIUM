#include "vmm.h"
#include "pmm.h"
#include "kernel/device/display/fprint.h"

static vmm_page_table_t *current_page_table = NULL;
static uintptr_t kernel_heap_end  = KERNEL_HEAP_START;

//helper function
void *my_memset(void *ptr, int value, size_t num) {
    unsigned char *p = ptr;
    while (num--) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}

//vmm functions 
void vmm_init(struct limine_memmap_response* memap){
    fprint("initing vmm/pmm\n");
    fprint("init pmm\n");
    pmm_init(memap);
    fprint("pmm init comppletetd during pmm test\n");
    fprint("allocating 2 pages for test\n");
    void *page1 = pmm_alloc_page();
    void *page2 = pmm_alloc_page();
    fprint("pages were allocated succesfully freeing pages \n");
    pmm_free_page(page1);
    fprint("pages freed from pmm making vmm page table for kernel\n");
    //kernel page table for kernel 
    size_t num_pages = 1024;
    current_page_table = (vmm_page_table_t*) pmm_alloc_page();
    fprint("making memset for vmm \n");
    my_memset(current_page_table,0,sizeof(vmm_page_table_t));
    //simple page table for kerbel in fist table
    fprint("making page for kernel only\n");
    current_page_table->entries =(pte_t *)pmm_alloc_page();
    my_memset(current_page_table->entries,0,PAGE_SIZE);
    fprint("was succesfull contining vmm init\n");
    fprint("kernel addres init\n");

    for (size_t i = 0; i < num_pages; i++) {
        set_pte(&current_page_table->entries[i], i * PAGE_SIZE, 0x3); // Zugriffsrechte: Read/Write
    }
}

//method to allocating virtual pages later needed for userspaces lol 
void *vmm_alloc_page(void){
    void *phys_addr = pmm_alloc_page();
    if (!phys_addr) return NULL;
    uintptr_t virt_addr = kernel_heap_end;
    kernel_heap_end += PAGE_SIZE;
    vmm_map_page((void*)virt_addr,phys_addr,0x3);
    
    return (void *)virt_addr;
}

//cleaning up pages 
void vmm_free_page(void* addr){
    vmm_unmap_page(addr);
    pmm_free_page(addr);
}

void* vmm_map_page(void *virt,void * phys,uint64_t flags){
    size_t virt_page = (uintptr_t)virt / PAGE_SIZE;
    size_t phys_page = (uintptr_t)phys / PAGE_SIZE;
    set_pte(&current_page_table->entries[virt_page], phys_page * PAGE_SIZE, flags); 

    return virt;
}


void vmm_unmap_page(void * virt){
    size_t virt_page = (uintptr_t)virt / PAGE_SIZE;
    set_pte(&current_page_table->entries[virt_page],0,0);

    return;
}

static inline void set_pte(pte_t *entry, uintptr_t phys_addr, uint64_t flags) {
    *entry = (phys_addr & ~0xFFF) | (flags & 0xFFF); 
}

static inline uintptr_t get_phys_addr_from_pte(pte_t *entry) {
    return *entry & ~0xFFF;
}


void vmm_load_page_table(vmm_page_table_t *page_table) {
    //place houlder
}