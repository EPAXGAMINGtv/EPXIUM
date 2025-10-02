#include "mouse.h"
#include "../display/fb.h"
#include "../display/colors.h"
#include "kernel/io/io.h"

#define MOUSE_IRQ 44

static mouse_state_t mouse_state;
static int8_t x_move, y_move;
static uint8_t buttons;


#define CURSOR_SIZE 5
static uint32_t cursor_backup[CURSOR_SIZE * CURSOR_SIZE];

void mouse_init(void) {
    mouse_state.x = 100;
    mouse_state.y = 100;
    mouse_state.left = 0;
    mouse_state.right = 0;
    mouse_state.middle = 0;
    mouse_state.scroll_up = 0;
    mouse_state.scroll_down = 0;

    x_move = 0;
    y_move = 0;
    buttons = 0;

    outb(0x64, 0xA8); 
    outb(0x64, 0x20); 
    uint8_t status = inb(0x60);
    status |= 2;
    outb(0x64, 0x60);
    outb(0x60, status);

    outb(0x64, 0xD4); 
    outb(0x60, 0xF4);
}

void mouse_poll(void) {
    uint8_t status = inb(0x64);
    if (status & 1) {
        uint8_t data = inb(0x60);
        static int cycle = 0;
        static uint8_t packet[3];
        packet[cycle++] = data;

        if(cycle == 3) {
            x_move = (int8_t)packet[1];
            y_move = (int8_t)packet[2];
            buttons = packet[0];

            mouse_state.x += x_move;
            mouse_state.y -= y_move;

            if(mouse_state.x < 0) mouse_state.x = 0;
            if(mouse_state.y < 0) mouse_state.y = 0;
            if(mouse_state.x > 1024) mouse_state.x = 1024;
            if(mouse_state.y > 768) mouse_state.y = 768;

            mouse_state.left   = buttons & 0x1;
            mouse_state.right  = (buttons & 0x2) >> 1;
            mouse_state.middle = (buttons & 0x4) >> 2;

            mouse_state.scroll_up   = (packet[0] & 0x08) >> 3;
            mouse_state.scroll_down = (packet[0] & 0x10) >> 4;

            cycle = 0;
        }
    }
}

mouse_state_t mouse_get_state(void) {
    return mouse_state;
}

static void backup_cursor_bg(uint32_t* fb_ptr, int fb_width, int fb_height, mouse_state_t* state) {
    int cx = state->x, cy = state->y;
    for(int dy = -CURSOR_SIZE/2; dy <= CURSOR_SIZE/2; dy++) {
        for(int dx = -CURSOR_SIZE/2; dx <= CURSOR_SIZE/2; dx++) {
            int px = cx + dx;
            int py = cy + dy;
            int idx = (dy + CURSOR_SIZE/2) * CURSOR_SIZE + (dx + CURSOR_SIZE/2);
            if(px >= 0 && px < fb_width && py >= 0 && py < fb_height)
                cursor_backup[idx] = fb_ptr[py * fb_width + px];
            else
                cursor_backup[idx] = COLOR_BLACK;
        }
    }
}


static void restore_cursor_bg(uint32_t* fb_ptr, int fb_width, int fb_height, mouse_state_t* state) {
    int cx = state->x, cy = state->y;
    for(int dy = -CURSOR_SIZE/2; dy <= CURSOR_SIZE/2; dy++) {
        for(int dx = -CURSOR_SIZE/2; dx <= CURSOR_SIZE/2; dx++) {
            int px = cx + dx;
            int py = cy + dy;
            int idx = (dy + CURSOR_SIZE/2) * CURSOR_SIZE + (dx + CURSOR_SIZE/2);
            if(px >= 0 && px < fb_width && py >= 0 && py < fb_height)
                fb_ptr[py * fb_width + px] = cursor_backup[idx];
        }
    }
}


static uint32_t cursor_backup[CURSOR_SIZE * CURSOR_SIZE];
static int old_x = 100, old_y = 100;

void draw_mouse_cursor(uint32_t* fb_ptr, int fb_width, int fb_height, mouse_state_t* state) {
    for(int dy=-CURSOR_SIZE/2; dy<=CURSOR_SIZE/2; dy++) {
        for(int dx=-CURSOR_SIZE/2; dx<=CURSOR_SIZE/2; dx++) {
            int px = old_x + dx;
            int py = old_y + dy;
            int idx = (dy+CURSOR_SIZE/2)*CURSOR_SIZE + (dx+CURSOR_SIZE/2);
            if(px>=0 && px<fb_width && py>=0 && py<fb_height)
                fb_ptr[py*fb_width + px] = cursor_backup[idx];
        }
    }

    for(int dy=-CURSOR_SIZE/2; dy<=CURSOR_SIZE/2; dy++) {
        for(int dx=-CURSOR_SIZE/2; dx<=CURSOR_SIZE/2; dx++) {
            int px = state->x + dx;
            int py = state->y + dy;
            int idx = (dy+CURSOR_SIZE/2)*CURSOR_SIZE + (dx+CURSOR_SIZE/2);
            if(px>=0 && px<fb_width && py>=0 && py<fb_height)
                cursor_backup[idx] = fb_ptr[py*fb_width + px];
        }
    }


    for(int dy=-CURSOR_SIZE/2; dy<=CURSOR_SIZE/2; dy++) {
        int py = state->y + dy;
        if(py>=0 && py<fb_height) fb_ptr[py*fb_width + state->x] = COLOR_LIGHT_GREEN;
    }
    for(int dx=-CURSOR_SIZE/2; dx<=CURSOR_SIZE/2; dx++) {
        int px = state->x + dx;
        if(px>=0 && px<fb_width) fb_ptr[state->y*fb_width + px] = COLOR_LIGHT_GREEN;
    }

    old_x = state->x;
    old_y = state->y;
}
