#include "process.h"

process_t *current = 0;
process_t *ready_queue = 0;

static process_t *last = 0;
extern void context_switch(process_t *old, process_t *new);

void process_init() {
    current = 0;
    ready_queue = 0;
    last = 0;
}

process_t* process_create(void (*entry)(), uint64_t stack_top) {
    static process_t proc_pool[16]; 
    static int idx = 0;
    if (idx >= 16) return 0;

    process_t *proc = &proc_pool[idx++];
    proc->rsp = stack_top;
    proc->rip = (uint64_t)entry;
    proc->state = PROC_READY;
    proc->next = 0;

    if (!ready_queue) {
        ready_queue = proc;
        last = proc;
    } else {
        last->next = proc;
        last = proc;
    }

    return proc;
}


void schedule() {
    if (!current) {
        current = ready_queue;
        return;
    }

    process_t *old = current;
    process_t *next = current->next ? current->next : ready_queue;
    current = next;

    context_switch(old, next);
}
