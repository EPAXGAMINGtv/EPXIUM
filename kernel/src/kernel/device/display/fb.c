#include "fb.h"
#include "kernel/font8x8/font8x8_basic.h"
#include "kernel/basic/wait.h"
#include <stdint.h>


void put_pixel_lim(volatile struct limine_framebuffer* fb, int x, int y, uint32_t color) {
    if (x < 0 || y < 0) return;
    if ((uint64_t)x >= fb->width || (uint64_t)y >= fb->height) return;

    uint8_t* pixel = (uint8_t*)fb->address + y * fb->pitch + x * (fb->bpp / 8);
    *(uint32_t*)pixel = color;
}

void clear_screen_lim(volatile struct limine_framebuffer* fb, uint32_t color) {
    for (uint64_t y = 0; y < fb->height; y++) {
        for (uint64_t x = 0; x < fb->width; x++) {
            put_pixel_lim(fb, x, y, color);
        }
    }
}
void draw_char_lim(volatile struct limine_framebuffer* fb, int x, int y, char c, uint32_t color) {
    for (int row = 0; row < 8; row++) {
        uint8_t bits = font8x8_basic[(int)c][row];
        for (int col = 0; col < 8; col++) {
            if (bits & (1 << col)) {
                put_pixel_lim(fb, x + col, y + row, color);
            }
        }
    }
}

void draw_text_lim(volatile struct limine_framebuffer* fb, int x, int y, const char* str, uint32_t color) {
    int cursor_x = x;
    while (*str) {
        if (*str == '\n') {
            cursor_x = x;
            y += 8;
        } else {
            draw_char_lim(fb, cursor_x, y, *str, color);
            cursor_x += 8;
        }
        str++;
    }
}



const char* letters[][5] = {
    {"  A  "," A A ","AAAAA","A   A","A   A"},
    {"BBBB ","B   B","BBBB ","B   B","BBBB "},
    {" CCC ","C   C","C    ","C   C"," CCC "},
    {"DDD  ","D  D ","D   D","D  D ","DDD  "},
    {"EEEEE","E    ","EEE  ","E    ","EEEEE"},
    {"FFFFF","F    ","FFF  ","F    ","F    "},
    {" GGG ","G    ","G  GG","G   G"," GGG "},
    {"H   H","H   H","HHHHH","H   H","H   H"},
    {"IIIII","  I  ","  I  ","  I  ","IIIII"},
    {"  JJJ","   J ","   J ","J  J "," JJ  "},
    {"K   K","K  K ","KKK  ","K  K ","K   K"},
    {"L    ","L    ","L    ","L    ","LLLLL"},
    {"M   M","MM MM","M M M","M   M","M   M"},
    {"N   N","NN  N","N N N","N  NN","N   N"},
    {" OOO ","O   O","O   O","O   O"," OOO "},
    {"PPPP ","P   P","PPPP ","P    ","P    "},
    {" QQQ ","Q   Q","Q   Q","Q  Q "," QQ Q"},
    {"RRRR ","R   R","RRRR ","R R  ","R  RR"},
    {" SSS ","S    "," SSS ","    S","SSSS "},
    {"TTTTT","  T  ","  T  ","  T  ","  T  "},
    {"U   U","U   U","U   U","U   U"," UUU "},
    {"V   V","V   V","V   V"," V V ","  V  "},
    {"W   W","W   W","W W W","WW WW","W   W"},
    {"X   X"," X X ","  X  "," X X ","X   X"},
    {"Y   Y"," Y Y ","  Y  ","  Y  ","  Y  "},
    {"ZZZZZ","   Z ","  Z  "," Z   ","ZZZZZ"},
    {" 000 ","0   0","0   0","0   0"," 000 "},
    {"  1  "," 11  ","  1  ","  1  "," 111 "},
    {" 222 ","2   2","  2  "," 2   ","22222"},
    {" 333 ","3   3","  33 ","3   3"," 333 "},
    {"4  4 ","4  4 ","44444","   4 ","   4 "},
    {"55555","5    ","5555 ","    5","5555 "},
    {" 666 ","6    ","6666 ","6   6"," 666 "},
    {"77777","   7 ","  7  "," 7   ","7    "},
    {" 888 ","8   8"," 888 ","8   8"," 888 "},
    {" 999 ","9   9"," 9999","    9"," 999 "},
};

int letter_index(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= '0' && c <= '9') return 26 + (c - '0');
    return -1;
}

void draw_scaled_char_lim(volatile struct limine_framebuffer* fb, int x, int y, const char* char_lines[5], uint32_t color, int scale) {
    for (int row = 0; row < 5; row++) {
        const char* line = char_lines[row];
        for (int col = 0; line[col]; col++) {
            if (line[col] != ' ')
                for (int dy = 0; dy < scale; dy++)
                    for (int dx = 0; dx < scale; dx++)
                        put_pixel_lim(fb, x + col * scale + dx, y + row * scale + dy, color);
        }
    }
}

void draw_square_lim(volatile struct limine_framebuffer* fb, int x, int y, int size, uint32_t color) {
    for (int yy = 0; yy < size; yy++)
        for (int xx = 0; xx < size; xx++)
            put_pixel_lim(fb, x + xx, y + yy, color);
}

void draw_boot_logo_lim(volatile struct limine_framebuffer* fb, int start_y,
                        const char* logo_name, uint32_t logo_color, int scale,
                        const char* under_text, uint32_t text_color) {
    int logo_length = 0;
    for (int i = 0; logo_name[i]; i++) logo_length += 6;
    int logo_pixel_width = logo_length * scale;
    int start_x_logo = (fb->width - logo_pixel_width) / 2;

    int x = start_x_logo;
    for (int i = 0; logo_name[i]; i++) {
        int idx = letter_index(logo_name[i]);
        if (idx >= 0) draw_scaled_char_lim(fb, x, start_y, letters[idx], logo_color, scale);
        x += 6 * scale;
    }

    int underline_y = start_y + 5 * scale;
    for (int px = start_x_logo; px < start_x_logo + logo_pixel_width; px++)
        for (int dy = 0; dy < scale / 2; dy++)
            put_pixel_lim(fb, px, underline_y + dy, logo_color);

    int text_len = 0;
    for (int i = 0; under_text[i]; i++) text_len++;
    int text_pixel_width = text_len * 8;
    int start_x_text = (fb->width - text_pixel_width) / 2;
    int text_y = underline_y + scale + 8;
    draw_text_lim(fb, start_x_text, text_y, under_text, text_color);

    int max_dots = 5;
    int dot_spacing = 15;
    int dot_size = 6;
    int start_x_dots = (fb->width - (max_dots * dot_spacing)) / 2;
    int dot_y = text_y + 26;

    for (int i = 0; i < max_dots; i++) {
        draw_square_lim(fb, start_x_dots + i * dot_spacing, dot_y, dot_size, text_color);
        wait_seconds(1);
    }
}
