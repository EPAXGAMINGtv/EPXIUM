#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include "screenmanagment/text.h"

// === Limine Requests ===

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;


static void hcf(void) {
    for (;;) {
        asm("hlt");
    }
}


size_t get_usable_ram_kb(void) {
    if (memmap_request.response == NULL) return 0;

    size_t total = 0;

    for (size_t i = 0; i < memmap_request.response->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap_request.response->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            total += entry->length;
        }
    }

    return total / 1024;  
}


void utoa(size_t value, char *str) {
    char tmp[20];
    int i = 0;
    if (value == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    while (value > 0) {
        tmp[i++] = '0' + (value % 10);
        value /= 10;
    }
    for (int j = 0; j < i; j++) {
        str[j] = tmp[i - j - 1];
    }
    str[i] = '\0';
}

char* strcat(char* dest, const char* src) {
    char* d = dest;
    while (*d) d++;
    while (*src) *d++ = *src++;
    *d = '\0';
    return dest;
}


void kmain(void) {
    if (!LIMINE_BASE_REVISION_SUPPORTED) hcf();
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) hcf();

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    draw_string(fb, 10 , 10, "Welcome to EPXIUM!", 0x00FF00);
    draw_string(fb, 10, 20, "Hey", 0x00FF00);


    size_t ram_kb = get_usable_ram_kb();

    char ram_str[64];
    char numbuf[20];

    utoa(ram_kb, numbuf); 

    ram_str[0] = '\0';
    strcat(ram_str, "Usable RAM: ");
    strcat(ram_str, numbuf);
    strcat(ram_str, " KB");

    draw_string(fb, 10, 30, ram_str, 0x00FF00);

    while (1) {
        asm("hlt");
    }
}
