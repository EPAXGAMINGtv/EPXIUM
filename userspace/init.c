// ...existing code...
#include <stdint.h>

volatile uint16_t* VGA_MEMORY = (uint16_t*)0xB8000;
const uint8_t VGA_WIDTH = 80;

static void print(const char* str, uint8_t row, uint8_t col, uint8_t color) {
    uint16_t* vga = (uint16_t*)( (uintptr_t)VGA_MEMORY + (row * VGA_WIDTH + col) * 2 );
    while (*str) {
        *vga++ = ((uint16_t)color << 8) | (uint8_t)(*str++);
    }
}

__attribute__((noreturn))
void _start() {
    print("Hello from userspace!", 0, 0, 0x0E);
    for (;;) {
        __asm__ volatile ("hlt" ::: "memory");
    }
}