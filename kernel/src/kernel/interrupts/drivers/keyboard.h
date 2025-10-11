#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

#define NUM_LAYOUTS 2 

#define KEYBOARD_BUFFER_SIZE 128
extern uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE];
extern uint8_t buffer_head;
extern uint8_t buffer_tail;

typedef enum{
    LAYOUT_US = 0,
    LAYOUT_DE = 1,
} key_layout_t;

#define KEY_A        0x1E
#define KEY_B        0x30
#define KEY_C        0x2E
#define KEY_D        0x20
#define KEY_E        0x12
#define KEY_F        0x21
#define KEY_G        0x22
#define KEY_H        0x23
#define KEY_I        0x17
#define KEY_J        0x24
#define KEY_K        0x25
#define KEY_L        0x26
#define KEY_M        0x32
#define KEY_N        0x31
#define KEY_O        0x18
#define KEY_P        0x19
#define KEY_Q        0x10
#define KEY_R        0x13
#define KEY_S        0x1F
#define KEY_T        0x14
#define KEY_U        0x16
#define KEY_V        0x2F
#define KEY_W        0x11
#define KEY_X        0x2D
#define KEY_Y        0x15
#define KEY_Z        0x2C

#define KEY_1        0x02
#define KEY_2        0x03
#define KEY_3        0x04
#define KEY_4        0x05
#define KEY_5        0x06
#define KEY_6        0x07
#define KEY_7        0x08
#define KEY_8        0x09
#define KEY_9        0x0A
#define KEY_0        0x0B

#define KEY_ENTER    0x1C
#define KEY_BACKSPACE 0x0E
#define KEY_TAB      0x0F
#define KEY_ESC      0x01
#define KEY_SPACE    0x39

#define KEY_F1       0x3B
#define KEY_F2       0x3C
#define KEY_F3       0x3D
#define KEY_F4       0x3E
#define KEY_F5       0x3F
#define KEY_F6       0x40
#define KEY_F7       0x41
#define KEY_F8       0x42
#define KEY_F9       0x43
#define KEY_F10      0x44
#define KEY_F11      0x57
#define KEY_F12      0x58

extern const uint8_t *keymap[NUM_LAYOUTS];

void init_keyboard(void);    
void keyboard_isr(void);
bool key_is_pressed(uint8_t keycode);
void clear_buffer(void);
void switch_layout(key_layout_t key_layout);
char scancode_to_char(uint8_t scancode);

#endif