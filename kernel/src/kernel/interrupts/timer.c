#include "timer.h"
#include "pic.h"
#include "kernel/io/io.h"
#include "kernel/device/display/fprint.h"

#define PIT_CMD_PORT 0x43
#define PIT_DATA_PORT 0x40

#define PIT_FREQUENCY 1193180
#define TIMER_IRQ 0

static uint64_t tick_count = 0; 

void timer_init(void){
    outb(0x36, PIT_CMD_PORT);

    uint16_t divisor = PIT_FREQUENCY /100;
    outb(divisor & 0xFF, PIT_DATA_PORT);  
    outb((divisor >> 8) & 0xFF, PIT_DATA_PORT);
    fprint("timer initzialzed [OK]\n");
}

void handle_timer(void){
    tick_count++;
    pic_send_eoi(0); 
    /*
    if (tick_count % 100 == 0) {
        //debug

    }
        */
        
}

void sleep(uint64_t ticks) {
    uint64_t start_tick = tick_count;
    while (tick_count - start_tick < ticks) {
        handle_timer();
        __asm__ volatile ("pause");  
    }
}

void sleep_ms(uint64_t ms){
    uint64_t miliseconds = 1000000;
    uint64_t result  = miliseconds * ms ; 
    sleep(result);
}

void sleep_s(uint64_t seconds){
    uint64_t Seconds = 10000000;
    uint64_t result = Seconds * seconds;
    sleep(Seconds);
}
