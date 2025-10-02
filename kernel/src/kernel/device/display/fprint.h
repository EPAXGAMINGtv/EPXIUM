#ifndef FPRINT_H
#define FPRINT_H

#include <stdint.h>
#include "cursor.h"
#include "fb.h"

typedef struct {
    volatile struct limine_framebuffer* fb;
    cursor_t* cursor;
    uint32_t color;
} fprint_t;

extern fprint_t g_printer;

void init_fprint_global(volatile struct limine_framebuffer* fb, cursor_t* cursor, uint32_t color);
void fprint(const char* text);

#endif
