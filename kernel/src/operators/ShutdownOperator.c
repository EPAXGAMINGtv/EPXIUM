#include "ShutdownOperator.h"

static inline void outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline void outw(unsigned short port, unsigned short val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void reboot(void) {
    while (inb(0x64) & 0x02);
    outb(0x64, 0xFE);
    for (;;) __asm__ volatile ("hlt");
}

void stop_os(void) {
    __asm__ volatile (
        "cli\n"
        "hlt\n"
    );
    for (;;) __asm__ volatile ("hlt");
}

void poweroff(void) {
    // Bochs & QEMU ACPI shutdown
    outw(0x604, 0x2000);

    // Alternative fallback: use "triple fault"
    outb(0xB004, 0x2000);

    // Halt if nothing worked
    for (;;) __asm__ volatile ("hlt");
}
