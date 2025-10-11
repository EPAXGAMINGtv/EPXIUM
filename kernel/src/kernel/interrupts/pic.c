#include "pic.h"
#include "../io/io.h"
#include "../device/display/fprint.h"

void pic_init(void){
    outb(ICW1_INIT | ICW1_IWC4, PIC1_CMD);
    outb(ICW1_INIT | ICW1_IWC4, PIC2_CMD);

    outb(0x20,PIC1_DATA);
    outb(0x20,PIC2_DATA);

    outb(0x04,PIC1_DATA);
    outb(0x04,PIC2_DATA);

    outb(ICW4_8086, PIC1_DATA);
    outb(ICW4_8086, PIC2_DATA);

    outb(0xFF,PIC1_DATA);
    outb(0xFF,PIC2_DATA);
    
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8) { 
        outb(0x20, PIC2_CMD); 
    }
    outb(0x20, PIC1_CMD); 
}


void pic_send_command(uint16_t port, uint8_t cmd) {
    outb(port, cmd);
}

void pic_send_data(uint16_t port, uint8_t data) {
    outb(port, data);
}
