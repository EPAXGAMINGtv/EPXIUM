#pragma once
#include <stdint.h>
#include <stddef.h>
#include <limine.h>

void draw_char(struct limine_framebuffer *fb, int x, int y, char c, uint32_t color);
void draw_string(struct limine_framebuffer *fb, int x, int y, const char* str, uint32_t color);
