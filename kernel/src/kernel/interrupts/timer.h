#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "isr.h"

void timer_init(void);
void handle_timer(void);
void sleep(uint64_t ticks);
void sleep_ms(uint64_t ms);
void sleep_s(uint64_t seconds);
void sleep_m(uint64_t minutes);
uint64_t get_tick_count();

#endif