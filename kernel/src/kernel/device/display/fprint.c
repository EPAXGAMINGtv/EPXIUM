#include "fprint.h"

fprint_t g_printer;

void init_fprint_global(volatile struct limine_framebuffer* fb, cursor_t* cursor, uint32_t color) {
    g_printer.fb = fb;
    g_printer.cursor = cursor;
    g_printer.color = color;
}

void fprint(const char* text) {
    const char* ptr = text;
    while (*ptr) {
        if (*ptr == '\n') {
            g_printer.cursor->x = 0;
            g_printer.cursor->y += g_printer.cursor->char_height;
        } else {
            put_char_with_cursor_lim(g_printer.fb, g_printer.cursor, *ptr, g_printer.color);
        }
        ptr++;
    }
}
