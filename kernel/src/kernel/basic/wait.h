#ifndef WAIT_H
#define WAIT_H

#include <stdint.h>

void wait_ms(uint32_t ms);       
void wait_seconds(uint32_t sec); 
void wait_minutes(uint32_t min); 
void wait_hours(uint32_t hr);    
void wait_days(uint32_t days);   
void wait(uint64_t ms);          

#endif
