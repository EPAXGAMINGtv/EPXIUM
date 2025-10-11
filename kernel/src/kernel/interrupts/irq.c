#include "irq.h"
#include "kernel/io/io.h"  
#include "kernel/device/display/fprint.h"
#include "idt.h" 
#include "gdt.h"

static irq_handler_t irq_handlers[MAX_IRQS];

#define PIC1_CMD 0x20  
#define PIC1_DATA 0x21 
#define PIC2_CMD 0xA0  
#define PIC2_DATA 0xA1 
#define EOI 0x20
void irq_init(void) {
    for (int i = 0; i < MAX_IRQS; i++) {
        irq_handlers[i] = irq_handler_default;
    }
}


void irq_register_handler(uint8_t irq, irq_handler_t handler) {
    if (irq < MAX_IRQS) {
        irq_handlers[irq] = handler;
    }
}

void irq_eoi(uint8_t irq) {
    if (irq >= 8) {
        outb(PIC2_CMD, EOI);  
    }
    outb(PIC1_CMD, EOI);  
}

void irq_handler_timer(void) {
    fprint("Timer interrupt executet\n");
    irq_eoi(0); 
}

void irq_handler_default(void) {
    fprint("interrupt without handler\n");
}
