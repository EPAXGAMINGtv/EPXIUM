#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

typedef enum { 
    PROC_READY, 
    PROC_RUNNING, 
    PROC_WAITING 
} proc_state_t;

typedef struct process {
    uint64_t rsp;          
    uint64_t rip;          
    uint64_t cr3;          
    proc_state_t state;    
    struct process *next;  
} process_t;


extern process_t *current;
extern process_t *ready_queue;

void process_init();
process_t* process_create(void (*entry)(), uint64_t stack_top);
void schedule();

#endif
