#include "isr.h"
#include "irq.h"
#include "timer.h"
#include "kernel/device/display/fprint.h"
#include "drivers/keyboard.h"
#include "kernel/io/io.h"

isr_handler_t isr_handlers[MAX_INTERRUPTS];

extern void keyboard_isr(void);

void isr_handler_default(void){
    fprint("unhandelt interrupts \n");
    while (1)
    {
        __asm__ volatile("hlt");
    }
    
}

/*void keyboard_isr(void) {
    uint8_t scancode = inb(0x60);
    keyboard_buffer[buffer_head] = scancode;
    buffer_head = (buffer_head + 1) % KEYBOARD_BUFFER_SIZE; 
}
    */

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

void exception_handler_divide_by_zero(void) {
    fprint("Kernel panic: Division by Zero!\n");
    while (1) {
        __asm__ volatile("hlt");
    }
}

void exception_handler_page_fault(void) {
    fprint("Kernel panic: Page Fault!\n");
    while (1) {
        __asm__ volatile("hlt");
    }
}

void isr_init(void){
    for (int i = 0; i< MAX_INTERRUPTS; i++){
        isr_handlers[i] = isr_handler_default;
    }

    register_isr_handler(32,handle_timer);
    register_isr_handler(33, keyboard_isr);
    register_isr_handler(0, exception_handler_divide_by_zero); 
     register_isr_handler(14, exception_handler_page_fault);
}