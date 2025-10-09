#include "gdt.h"
#include "../device/display/fprint.h"

#define GDT_ENTRIES 3 

gdt_entry_t gdt[GDT_ENTRIES];
gdt_ptr_t gdt_ptr;

void gdt_set_gate(int n, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity) {
    gdt[n].base_low = base & 0xFFFF;
    gdt[n].base_middle = (base >> 16) & 0xFF;
    gdt[n].base_high = (base >> 24) & 0xFF;

    gdt[n].limit_low = limit & 0xFFFF;
    gdt[n].granularity = (limit >> 16) & 0x0F;

    gdt[n].granularity |= granularity & 0xF0;
    gdt[n].access = access;
}


void gdt_init(void) {
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uint32_t)&gdt;
    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);  
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);  
    __asm__ volatile("lgdt (%0)" : : "r" (&gdt_ptr));
    fprint("gdt initzialed [OK]\n");
}
