#include "wait.h"

static inline void delay_cycles(uint64_t cycles) {
    volatile uint64_t i;
    for(i = 0; i < cycles; i++) {
        __asm__ volatile("nop");
    }
}

#define CYCLES_PER_MS 100000ULL

void wait_ms(uint32_t ms) {
    delay_cycles((uint64_t)ms * CYCLES_PER_MS);
}

void wait_seconds(uint32_t sec) {
    wait_ms(sec * 1000);
}

void wait_minutes(uint32_t min) {
    wait_seconds(min * 60);
}

void wait_hours(uint32_t hr) {
    wait_minutes(hr * 60);
}

void wait_days(uint32_t days) {
    wait_hours(days * 24);
}

void wait(uint64_t ms) {
    wait_ms((uint32_t)ms);
}
