#include "keyboard.h"
#include "kernel/io/io.h"
#include "kernel/device/display/fprint.h"

uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE];
uint8_t buffer_head = 0;
uint8_t buffer_tail = 0;

static const uint8_t keymap_us[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t', 
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0, 'a', 's', 
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c', 'v', 
    'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    // function keys (F1-F12)
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
};

static const uint8_t keymap_de[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'ß', '´', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 'ü', '+', '\n', 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', 'ö', 'ä', '#', 0, '\\', 'y', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '-', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0,
    // Function keys (F1-F12)
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

const uint8_t *keymap[NUM_LAYOUTS] = {keymap_us, keymap_de};

void keyboard_isr(void) {
    uint8_t status = inb(0x64);  
    if (status & 0x01) {  
        uint8_t scancode = inb(0x60);  
        fprint("key pressed\n");
        if (((buffer_head + 1) % KEYBOARD_BUFFER_SIZE) != buffer_tail) {
            keyboard_buffer[buffer_head] = scancode;  
            buffer_head = (buffer_head + 1) % KEYBOARD_BUFFER_SIZE;
        }
    }
}


void switch_layout(key_layout_t key_layout){
    if (key_layout<NUM_LAYOUTS)
    {
        keymap[key_layout]=keymap[key_layout];
    }
    
}

char scancode_to_char(uint8_t scancode){
    if (scancode < 128)
    {
        return keymap[LAYOUT_DE][scancode];
    }
}

bool key_is_pressed(uint8_t keycode) {
    for (int i = buffer_tail; i != buffer_head; i = (i + 1) % KEYBOARD_BUFFER_SIZE) {
        if (keyboard_buffer[i] == keycode) {
            return true;
        }
    }
    return false;
}


void clear_buffer(void) {
    buffer_head = 0;
    buffer_tail = 0;
}

void init_keyboard(void) {

    clear_buffer();  
}