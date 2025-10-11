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
void fprintcolor(const char* text, uint32_t color);
void fprint_int(int num);            
void fprint_uint(uint32_t num);      
void fprinthex(uint32_t num);        
void fprintcolor_int32(int num, uint32_t color);   
void fprintcolor_uint32(uint32_t num, uint32_t color); 
void fprintcolorhex(uint32_t num, uint32_t color);  

#endif
