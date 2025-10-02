#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include "kernel/device/display/fb.h"       
#include "kernel/device/display/colors.h"    
#include "kernel/device/display/fprint.h"    
#include "kernel/device/display/cursor.h"    
#include "kernel/basic/wait.h"               
#include "kernel/device/keyboard/keyboard.h" 
#include "kernel/operator/shutdown.h"        
#include "kernel/io/io.h"                    
#include "kernel/filesystem/FAT32/fat32.h"
#include "kernel/device/mouse/mouse.h"       



#define SHELL_MAX_INPUT 128  
#define CURSOR_BLINK_LOOPS 8000000  

static char input_buffer[SHELL_MAX_INPUT];  
static int input_len = 0;  

static volatile struct limine_framebuffer* framebuffer;  
static int fb_width, fb_height;  
static cursor_t shell_cursor_struct;  

int cursor_visible = 1;  
int blink_counter = 0;  
cursor_t* shell_cursor;  

uint32_t color = COLOR_NEON_GREEN;  

__attribute__((used, section(".limine_requests")))
volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};


#define MATRIX_WIDTH  200  
#define MATRIX_HEIGHT 10  

static unsigned int seed = 12345;
unsigned int simple_rand() {
    seed = seed * 1103515245 + 12345;
    return (seed / 65536) % 32768;
}

void simple_srand(unsigned int new_seed) {
    seed = new_seed;
}

void render_matrix() {
    char matrix[MATRIX_WIDTH][MATRIX_HEIGHT];  
    char possible_chars[] = {'0', '1', '0', '1', '101', '   ', '    ', '    ', '                                 ', '   '};  
    for (int col = 0; col < MATRIX_WIDTH; col++) {
        for (int row = 0; row < MATRIX_HEIGHT; row++) {
            int random_index = simple_rand() % (sizeof(possible_chars) / sizeof(possible_chars[0]));
            matrix[col][row] = possible_chars[random_index];
        }
    }


    for (int row = 0; row < MATRIX_HEIGHT; row++) {
        for (int col = 0; col < MATRIX_WIDTH; col++) {
            char c_str[2] = {matrix[col][row], '\0'};  
            fprint(c_str);  
        }
        fprint("\n");  
    }
}



void print_logo(){
    fprint("EEEEE   PPPPP   XX    XX    IIIIII     UU    UU   MM   MM\n");
    fprint("EE      PP  PP    XX XX       II       UU    UU   MMM MMM\n");
    fprint("EEEE    PPPPP      XX         II       UU    UU   MM M MM\n");
    fprint("EE      PP        XX XX       II       UU    UU   MM   MM\n");
    fprint("EEEEE   PP      XX    XX    IIIIII       UUUUU    MM   MM\n");
    fprint("---------------------------------------------------------\n");
    fprint("\n");
    fprint("---------------------------------------------------------\n");
    fprint("\n");
    fprint("This is EPXIUM an Homemade Operation System using limine\n");
    fprint("if you need  help do help in Command line!\n");
}

static int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) a++, b++;
    return *(unsigned char*)a - *(unsigned char*)b;
}

static int strlen(const char* s) {
    int len = 0;
    while (*s++) len++;
    return len;
}


static void shell_clear_screen() {
    clear_screen_lim(framebuffer, 0x000000);  
    shell_cursor->x = 0;  
    shell_cursor->y = 100;  
    print_logo();
}

static void shell_backspace() {
    if (input_len > 0) {
        input_len--;  
        input_buffer[input_len] = '\0';  

        if (cursor_visible) clear_cursor_lim(framebuffer, shell_cursor);  
        if (shell_cursor->x >= shell_cursor->char_width) {
            shell_cursor->x -= shell_cursor->char_width;  
        } else if (shell_cursor->y > 0) {
            shell_cursor->y -= shell_cursor->char_height;  
            shell_cursor->x = fb_width - shell_cursor->char_width;  
        }

        draw_char_lim(framebuffer->address, fb_width, shell_cursor->x, shell_cursor->y, 0x000000);  

        cursor_visible = 1;
        render_cursor_lim(framebuffer, shell_cursor, color);  
    }
}


static void shell_print_prompt() {
    const char* prompt = "<os $> :";  
    for (int i = 0; prompt[i]; i++)
        put_char_with_cursor_lim(framebuffer, shell_cursor, prompt[i], color);  
}

static void shell_clear_input() {
    input_len = 0;
    input_buffer[0] = '\0';  
}

static int strncmp(const char* a, const char* b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i] || a[i] == '\0' || b[i] == '\0')
            return (unsigned char)a[i] - (unsigned char)b[i];
    }
    return 0;
}


static void shell_execute_command(const char* cmd) {
    if (strcmp(cmd, "help") == 0) {
        fprint("Commands:\n  help     - show this help\n  clear    - clear screen\n  reboot   - reboot the system\n  shutdown - power off system\n  version  - shows os version\n  cmatrix  - makes matrix effeckt\n  echo     - prints whta your write in the back opf it \n");
    } else if (strcmp(cmd, "clear") == 0) {
        shell_clear_screen();  
    } else if (strcmp(cmd, "reboot") == 0) {
        fprint("Rebooting in 3 seconds...\n");
        wait_seconds(2);  
        fprint("Rebooting in 2 seconds...\n");
        wait_seconds(2);  
        fprint("Rebooting in 1 second...\n");
        wait_seconds(2);  
        outb(0x64, 0xFE);  
        for (;;) __asm__ volatile("hlt");
    } else if (strcmp(cmd, "shutdown") == 0) {
        fprint("Shutting down in 5 seconds!\n");
        wait_seconds(5);  
        shutdown();  
        for (;;) __asm__ volatile("hlt");  
    } else if (strcmp(cmd, "version") == 0) {
        fprint("version 0.0.1 build 15\n");
    } else if (strncmp(cmd, "echo ", 5) == 0) {
        fprint(cmd + 5); 
        fprint("\n");
    }else if (strncmp(cmd, "cmatrix", 5) == 0) {
        int time =0;
        while(time<cmd){
        render_matrix();
        time++;
        }
    }else if (strcmp(cmd, "printfile") == 0) {
    uint8_t buffer[512];  
    uint8_t filename[11];
    int is_dir;

    uint32_t parent_cluster = fat32_resolve_path((uint8_t*)"sysroot/test.txt", filename, &is_dir);
    if (parent_cluster == 0 || is_dir) {
        fprint("Datei nicht gefunden oder ist ein Verzeichnis\n");
        return;
    }

    uint32_t current_cluster = parent_cluster;
    uint32_t next_cluster;
    size_t total_read = 0;
    size_t bytes_read = 0;

    while (current_cluster < 0x0FFFFFF8) {
        size_t remaining = sizeof(buffer) - total_read;
        if (remaining == 0) break;

        bytes_read = fat32_read(current_cluster, buffer + total_read, remaining);
        if (bytes_read == 0) {
            fprint("Fehler beim Lesen der Datei oder Ende der Datei erreicht\n");
            break; 
        }

        fprint("Gelesene Bytes: ");
        fprint("\n");

        total_read += bytes_read;

        next_cluster = fat32_get_next_cluster(current_cluster);
        fprint("\n");

        if (next_cluster == 0) {
            fprint("Ende der Clusterverkettung erreicht\n");
            break;
        }

        current_cluster = next_cluster;
    }

    if (total_read > 0) {
        buffer[total_read] = '\0';  
        fprint("Gelesene Datei:\n");
        fprint((char*)buffer);  
    } else {
        fprint("Die Datei ist leer oder konnte nicht gelesen werden\n");
    }
}


      else if (strlen(cmd) > 0) {
        fprint("Unknown command: ");
        fprint(cmd);  
        fprint("\n");
    }
}

static void shell_handle_key(char c) {
    if (cursor_visible) clear_cursor_lim(framebuffer, shell_cursor);

    if (c == '\n') {  
        input_buffer[input_len] = '\0';  
        fprint("\n");
        shell_execute_command(input_buffer);
        shell_clear_input();  
        shell_print_prompt();  
    } else if (c == '\b') {  
        shell_backspace();  
    } else if (c >= 32 && c < 127) {  
        if (input_len < SHELL_MAX_INPUT - 1) {
            input_buffer[input_len++] = c;  
            put_char_with_cursor_lim(framebuffer, shell_cursor, c, color);  
        }
    }
    cursor_visible = 1;
    render_cursor_lim(framebuffer, shell_cursor, color);
}

static void update_cursor_blink() {
    blink_counter++;
    if (blink_counter >= CURSOR_BLINK_LOOPS) {
        blink_counter = 0;
        if (cursor_visible) {
            clear_cursor_lim(framebuffer, shell_cursor);
            cursor_visible = 0;
        } else {
            render_cursor_lim(framebuffer, shell_cursor, color);
            cursor_visible = 1;
        }
    }
}

static void shell_run() {
    shell_print_prompt();  
    while (1) {
        keyboard_poll();  
        char c;
        if (keyboard_getchar(&c))
            shell_handle_key(c);  

        update_cursor_blink();  
    }
}



void kmain(void) {
    framebuffer = framebuffer_request.response->framebuffers[0];  
    fb_width = framebuffer->width;
    fb_height = framebuffer->height;

    init_cursor(&shell_cursor_struct, fb_width-2, fb_height-10);  
    shell_cursor = &shell_cursor_struct;  
    shell_cursor->y = 70;  

    init_fprint_global(framebuffer, shell_cursor, color);

    uint16_t flags;
    asm volatile("pushf; pop %0" : "=rm"(flags));  
    if (flags & (1 << 9)) {
    fprint("Interrupts are activated.\n");
    } else {
    fprint("Interrupts are deactivated.\n");
    }


    fprint("Before enabling interrupts\n");
    asm volatile("cli");
    fprint("Interrupts disabled, proceeding with initialization\n");
    

    draw_boot_logo_lim(framebuffer, fb_height / 4, "EPXIUM", COLOR_NEON_GREEN, 30, 
                       "This is EPXIUM, a homemade OS by EPAXGAMING", COLOR_NEON_GREEN);

    shell_clear_screen();
    



    keyboard_init();  
    
    shell_run();
}
