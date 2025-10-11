#include "fprint.h"

fprint_t g_printer;

void itoa(int num, char* str, int base) {
    int i = 0;
    int isNegative = 0;
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
    if (num < 0 && base == 10) {
        isNegative = 1;
        num = -num;
    }
    while (num != 0) {
        int rem = num % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        num = num / base;
    }
    if (isNegative) {
        str[i++] = '-';
    }
    str[i] = '\0'; 
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
    }
}


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

void fprintcolor(const char* text,uint32_t colooor){
        const char* ptr = text;
    while (*ptr) {
        if (*ptr == '\n') {
            g_printer.cursor->x = 0;
            g_printer.cursor->y += g_printer.cursor->char_height;
        } else {
            put_char_with_cursor_lim(g_printer.fb, g_printer.cursor, *ptr, colooor);
        }
        ptr++;
    }
}

void fprint_int(int num) {
    char buffer[20]; 
    itoa(num, buffer, 10); 
    fprint(buffer); 
}

void fprint_uint(uint32_t num) {
    char buffer[20]; 
    itoa(num, buffer, 10); 
    fprint(buffer); 
}

void fprinthex(uint32_t num) {
    char buffer[20];
    itoa(num, buffer, 16); 
    fprint("0x");
    fprint(buffer);
}


void fprintcolor_int32(int num, uint32_t color) {
    char buffer[20];
    itoa(num, buffer, 10); 
    fprintcolor(buffer, color); 
}

void fprintcolor_uint32(uint32_t num, uint32_t color) {
    char buffer[20];
    itoa(num, buffer, 10); 
    fprintcolor(buffer, color); 
}


void fprintcolorhex(uint32_t num, uint32_t color) {
    char buffer[20];
    itoa(num, buffer, 16); 
    fprintcolor("0x", color); 
    fprintcolor(buffer, color); 
}