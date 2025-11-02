#include "kmalloc.h"
#include "vmm.h"
#include "pmm.h"
#include "kernel/device/display/fprint.h"

#define KERNEL_HEAP_START 0xE0000000      // Start address of the kernel heap
#define KERNEL_HEAP_INITIAL_SIZE 0x100000 // Initial heap size: 1 MiB

// Block header struct for each allocation
typedef struct kmalloc_block {
    size_t size;               // Size of this block (including header)
    struct kmalloc_block *next; // Next free block
} kmalloc_block_t;

// Heap pointers
static uintptr_t heap_start = KERNEL_HEAP_START;
static uintptr_t heap_end   = KERNEL_HEAP_START;
static uintptr_t heap_max   = KERNEL_HEAP_START + KERNEL_HEAP_INITIAL_SIZE;

// Free list head
static kmalloc_block_t *free_list = NULL;

// Align a size to 8 bytes for better memory alignment
static inline size_t align8(size_t size) {
    return (size + 7) & ~7;
}

// Allocate more pages if needed
static void expand_heap(size_t size) {
    while (heap_end + size > heap_max) {
        void *phys = pmm_alloc_page();
        if (!phys) {
            fprint("kmalloc: out of physical memory!\n");
            return;
        }
        vmm_map_page((void *)heap_max, phys,0x3);
        heap_max += PAGE_SIZE;
    }
}

// Initialize kernel heap
void kmalloc_init(void) {
    fprint("Initializing kernel heap...\n");
    heap_start = KERNEL_HEAP_START;
    heap_end   = KERNEL_HEAP_START;
    heap_max   = KERNEL_HEAP_START + KERNEL_HEAP_INITIAL_SIZE;
    free_list  = NULL;
    fprint("Kernel heap initialized.\n");
}

// Allocate memory
void *kmalloc(size_t size) {
    if (size == 0) return NULL;

    size = align8(size); // Align to 8 bytes
    kmalloc_block_t **prev = &free_list;
    kmalloc_block_t *curr = free_list;
    while (curr) {
        if (curr->size >= size) {
            *prev = curr->next; 
            return (void *)(curr + 1); 
        }
        prev = &curr->next;
        curr = curr->next;
    }
    expand_heap(size + sizeof(kmalloc_block_t));
    kmalloc_block_t *block = (kmalloc_block_t *)heap_end;
    block->size = size;
    heap_end += size + sizeof(kmalloc_block_t);

    return (void *)(block + 1);
}

// Free memory
void kfree(void *ptr) {
    if (!ptr) return;

    kmalloc_block_t *block = (kmalloc_block_t *)ptr - 1; 
    block->next = free_list;  
    free_list = block;
}
