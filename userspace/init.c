#include <stdint.h>

// VGA Text Mode Speicheradresse
volatile uint16_t* VGA_MEMORY = (uint16_t*)0xB8000;
uint8_t VGA_WIDTH = 80;

// Einfache Funktion zum Schreiben eines Strings auf den Bildschirm
void print(const char* str, uint8_t row, uint8_t col, uint8_t color) {
    uint16_t* vga = VGA_MEMORY + row * VGA_WIDTH + col;
    while (*str) {
        *vga++ = ((uint16_t)color << 8) | *str++;
    }
}

// Entry Point
void _start() {
    // Schreibe "Hello from userspace!" in hellgelb auf schwarzen Hintergrund
    print("Hello from userspace!", 0, 0, 0x0E);

    // Endlosschleife, damit das Programm nicht abstürzt
    while (1) {}
}
