#include "isr.h"
#include "irq.h"
#include "timer.h"
#include "kernel/device/display/fprint.h"

isr_handler_t isr_handlers[MAX_INTERRUPTS];

void isr_handler_default(void){
    fprint("unhandelt interrupts \n");
    while (1)
    {
        __asm__ volatile("hlt");
    }
    
}


void register_isr_handler(uint8_t irq,isr_handler_t handler){
   if (irq < MAX_INTERRUPTS) {
        isr_handlers[irq] = handler;
    }
}

void isr_handler(uint8_t irq){
    if (irq < MAX_INTERRUPTS && isr_handlers[irq] != 0)
    {
        isr_handlers[irq]();
    }else{
        isr_handler_default();
    }
    
}

void exeption_handler_divide_by_zero(void){
    fprint("Kernel panic : Division by Zero!\n");
}

void isr_init(void){
    for (int i = 0; i< MAX_INTERRUPTS; i++){
        isr_handlers[i] = isr_handler_default;
    }

    register_isr_handler(32,handle_timer);
}