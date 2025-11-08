#include "userspace.h"
#include "kernel/fs/ramfs.h"
#include "kernel/device/display/fprint.h"
#include "kernel/mem/vmm.h"
#include <stdint.h>

#define USER_STACK_PAGES 8
#define USER_DS 0x23
#define USER_CS 0x1B

void userspace_load(const char* path) {
    if (!path) {
        fprint("userspace_load: path is NULL\n");
        return;
    }

    size_t size;
    void* elf_data = ramfs_read_file_to_memory(path, &size);
    if (!elf_data || size < ELF64_ENTRY_OFFSET + 8) {
        fprint("Failed to load userspace or ELF too small\n");
        return;
    }

    uint64_t entry = *((uint64_t*)((uint8_t*)elf_data + ELF64_ENTRY_OFFSET));
    if (!entry) {
        fprint("userspace_load: entry point is NULL\n");
        return;
    }

    userspace_entry_t main_entry = (userspace_entry_t)entry;

    fprint("Userspace loaded, jumping...\n");
    userspace_start(main_entry);
}

void userspace_start(userspace_entry_t entry) {
    if (!entry) return;

    void *stack_top_ptr = vmm_alloc_stack(USER_STACK_PAGES);
    uintptr_t user_stack;

    if (stack_top_ptr) {
        user_stack = (uintptr_t)stack_top_ptr;
    } else {
        void *stack_base_ptr = vmm_alloc_user_pages(USER_STACK_PAGES);
        if (!stack_base_ptr) {
            fprint("userspace_start: failed to allocate stack\n");
            return;
        }
        user_stack = (uintptr_t)stack_base_ptr + (uintptr_t)USER_STACK_PAGES * (uintptr_t)PAGE_SIZE;
    }

    user_stack &= ~((uintptr_t)0xF);

    fprint("Jumping to userspace...\n");

    __asm__ volatile (
        "mov %[ds16], %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %[stack64], %%rax\n"
        "pushq %[ss64]\n"
        "pushq %%rax\n"
        "pushfq\n"
        "orq $0x200, (%%rsp)\n"
        "pushq %[cs64]\n"
        "pushq %[entry64]\n"
        "iretq\n"
        :
        : [ds16]   "r"((uint16_t)USER_DS),
          [ss64]   "r"((uint64_t)USER_DS),
          [cs64]   "r"((uint64_t)USER_CS),
          [stack64]"r"(user_stack),
          [entry64]"r"(entry)
        : "rax", "memory"
    );
}