#ifndef USERSPACE_H
#define USERSPACE_H

#include <stdint.h>
#include <stddef.h>

#define ELF64_ENTRY_OFFSET 0x18


typedef void (*userspace_entry_t)(void);

void userspace_load(const char* path);
void userspace_start(userspace_entry_t entry);

#endif 
