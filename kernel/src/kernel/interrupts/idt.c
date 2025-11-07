#include "idt.h"
#include "gdt.h"  
#include "timer.h"
#include "../device/display/fprint.h"
#include "isr.h"

#define IDT_ENTRIES 256

idt_entry_t idt[IDT_ENTRIES];
idt_ptr_t idt_ptr;

extern void handle_timer(void);
extern void exception_handler_divide_by_zero(void); 
extern void exception_handler_page_fault(void);
extern void keyboard_isr(void);

void idt_set_gate(int n, uint64_t handler, uint16_t selector, uint8_t type_attr) {
    idt[n].offset_low = handler & 0xFFFF;
    idt[n].selector = selector;
    idt[n].ist = 0; 
    idt[n].type_attr = type_attr;
    idt[n].offset_mid = (handler >> 16) & 0xFFFF;
    idt[n].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[n].zero = 0;
}

void lidt(idt_ptr_t* idt_ptr) {
    __asm__ volatile ("lidt (%0)" : : "r" (idt_ptr));
}

void idt_init(void) {
    gdt_init();  
    idt_ptr.limit = (sizeof(idt_entry_t) * IDT_ENTRIES) - 1;
    idt_ptr.base = (uint64_t)&idt;
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    idt_set_gate(32, (uint64_t)handle_timer, 0x08, 0x8E);  
    idt_set_gate(33, (uint64_t)keyboard_isr, 0x08, 0x8E);  
    idt_set_gate(0, (uint64_t)exception_handler_divide_by_zero, 0x08, 0x8E);  
    idt_set_gate(14, (uint64_t)exception_handler_page_fault, 0x08, 0x8E);


    lidt(&idt_ptr);
}