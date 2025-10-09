#ifndef PIC_H
#define PIC_H

#include <stdint.h>

#define PIC1_CMD 0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

#define ICW1_INIT 0x10
#define ICW1_IWC4 0x01
#define ICW4_8086 0x01 


void pic_init(void);
void pic_send_eoi(uint8_t irq);

#endif