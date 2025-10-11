#ifndef IRQ_H
#define IRQ_H

#include <stdint.h>

#define MAX_IRQS 16


typedef void (*irq_handler_t)(void);
void irq_init(void);
void irq_register_handler(uint8_t irq, irq_handler_t handler);
void irq_handler_default(void);
void irq_handler_timer(void);
void irq_eoi(uint8_t irq);

#endif 
