#include "time/timer.h"
#include <stdint.h>

// I/O-Funktion: Ein Byte an Port senden
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

// PIT-Konstanten
#define PIT_FREQUENCY 1193182
#define PIT_COMMAND_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40
#define IRQ0 32  // Nach PIC-Remap IRQ0 liegt auf Vektor 32

// Globale Tick-Zählung (vom Timer-Interrupt erhöht)
static volatile uint64_t timer_ticks = 0;

// Externer Interrupt-Handler-Registrierer (muss definiert sein)
extern void register_interrupt_handler(uint8_t vector, void (*handler)(void));

// Timer Interrupt Handler (wird bei IRQ0 aufgerufen)
void pit_interrupt_handler(void) {
    timer_ticks++;
    // EOI (End of Interrupt) an PIC muss extern gesendet werden,
    // z.B. in deinem PIC-Code oder direkt hier (falls gewünscht)
}

// Timer initialisieren mit gewünschter Frequenz (Hz)
void timer_init(uint32_t frequency) {
    uint16_t divisor = (uint16_t)(PIT_FREQUENCY / frequency);

    // PIT auf Kanal 0, Zugriff LSB/MSB, Modus 3 (Square Wave Generator), Binär
    outb(PIT_COMMAND_PORT, 0x36);

    // Divisor senden (LSB zuerst, dann MSB)
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));        // LSB
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF)); // MSB

    // Timer Interrupt Handler registrieren (IRQ0 liegt auf Vektor 32)
    register_interrupt_handler(IRQ0, pit_interrupt_handler);
}

// Wartefunktion: Schlafe für ms Millisekunden (busy wait mit hlt)
void sleep(uint64_t ms) {
    uint64_t end = timer_ticks + ms;
    while (timer_ticks < end) {
        __asm__ volatile ("hlt");
    }
}

// Getter für aktuelle Anzahl der Ticks
uint64_t timer_get_ticks(void) {
    return timer_ticks;
}
