#include "pic.h"
#include "../io/io.h"
#include "../device/display/fprint.h"

void pic_init(void) {
    outb(ICW1_INIT | ICW1_IWC4, PIC1_CMD);  // Initialisierung des PIC1
    outb(ICW1_INIT | ICW1_IWC4, PIC2_CMD);  // Initialisierung des PIC2

    outb(0x20, PIC1_DATA);  // IRQ0-7 auf Vektoren 0x20-0x27 setzen
    outb(0x28, PIC2_DATA);  // IRQ8-15 auf Vektoren 0x28-0x2F setzen

    outb(0x04, PIC1_DATA);  // PIC1 weiß, dass PIC2 auf IRQ2 liegt
    outb(0x02, PIC2_DATA);  // PIC2 weiß, dass er mit PIC1 verbunden ist

    outb(ICW4_8086, PIC1_DATA);  // 8086-Modus für beide PICs
    outb(ICW4_8086, PIC2_DATA);

    outb(0xFF, PIC1_DATA);  // Alle IRQs auf PIC1 maskieren
    outb(0xFF, PIC2_DATA);  // Alle IRQs auf PIC2 maskieren
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
