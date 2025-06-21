#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void timer_init(uint32_t frequency);
void sleep(uint64_t ms);
uint64_t timer_get_ticks(void);

#ifdef __cplusplus
}
#endif
