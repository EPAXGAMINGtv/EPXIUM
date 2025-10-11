#ifndef ISR_H
#define ISR_H

#include <stdint.h>
#include "irq.h"

#define MAX_INTERRUPTS 256



typedef void (*isr_handler_t)(void);

extern isr_handler_t isr_handlers[MAX_INTERRUPTS];

void register_isr_handler(uint8_t irq,isr_handler_t handler);
void isr_handler_default(void);
void isr_init(void);
void isr_handler(uint8_t irq);
void exeption_handler_divide_by_zero(void);


#endif