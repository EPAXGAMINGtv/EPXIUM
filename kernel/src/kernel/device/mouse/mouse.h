#ifndef MOUSE_H
#define MOUSE_H

#include <stdint.h>


typedef struct {
    int x;              
    int y;              
    uint8_t left;       
    uint8_t right;      
    uint8_t middle;     
    uint8_t scroll_up;  
    uint8_t scroll_down
} mouse_state_t;

void mouse_init(void);
void mouse_poll(void);               // Polling
mouse_state_t mouse_get_state(void); 
void draw_mouse_cursor(uint32_t* fb_ptr, int fb_width, int fb_height, mouse_state_t* state);

#endif
