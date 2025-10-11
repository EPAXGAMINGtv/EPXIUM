#include <stdint.h>
#include <stddef.h>
#include <limine.h>
#include "kernel/device/display/fb.h"       
#include "kernel/device/display/colors.h"    
#include "kernel/device/display/fprint.h"    
#include "kernel/device/display/cursor.h"            
#include "kernel/operator/shutdown.h"        
#include "kernel/io/io.h"                           
#include "kernel/interrupts/gdt.h"
#include "kernel/interrupts/idt.h"
#include "kernel/interrupts/timer.h"
#include "kernel/interrupts/pic.h"
#include "kernel/interrupts/irq.h"
#include "kernel/interrupts/isr.h"
#include "kernel/mem/pmm.h"
#include "kernel/mem/vmm.h"



#define SHELL_MAX_INPUT 128  
#define CURSOR_BLINK_LOOPS 8000000  

static char input_buffer[SHELL_MAX_INPUT];  
static int input_len = 0;  

static volatile struct limine_framebuffer* framebuffer;  
static int fb_width, fb_height;  
static cursor_t shell_cursor_struct;  

cursor_t* shell_cursor;  

uint32_t color = COLOR_WHITE;  

__attribute__((used, section(".limine_requests")))
volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};



static void itoa(size_t value, char* str) {
    int i = 0;
    int negative = 0;
    if (value == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }
    if (value < 0) {
        negative = 1;
        value = -value;
    }
    while (value != 0) {
        str[i++] = (value % 10) + '0';
        value /= 10;
    }
    if (negative) {
        str[i++] = '-';
    }
    str[i] = '\0';
    int start = 0;
    int end = i - 1;
    char temp;
    while (start < end) {
        temp = str[start];
        str[start] = str[end];
        str[end] = temp;
        start++;
        end--;
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

}
void printOK(const char* beforetext){
    fprint(beforetext);
    fprint("[");
    fprintcolor("OK",COLOR_NEON_GREEN);
    fprint("]\n");
}

void printERR(const char* beforetext){
    fprint(beforetext);
    fprint("[");
    fprintcolor("ERROR",COLOR_RED);
    fprint("]\n");
}




void init_interrupts(void){ 
    printOK("initing interrupts ");
    printOK("init gdt ");
    gdt_init();
    printOK("gdt initzialized ");
    printOK("initing idt ");
    idt_init();
    printOK("idt initzialized ");
    
    printOK("init pic ");
    pic_init();
    outb(0xFE, PIC1_DATA);  
    outb(0xFF, PIC2_DATA);  
    printOK("pic initzialized ");
    printOK("timer init ");

    irq_init();

    timer_init();
    printOK("timer initzialized");
    isr_init();
    __asm__ volatile("sti");
    printOK("interrupts init completet");
}

void fb_init(void){
    framebuffer = framebuffer_request.response->framebuffers[0];  
    fb_width = framebuffer->width;
    fb_height = framebuffer->height;

    init_cursor(&shell_cursor_struct, fb_width-2, fb_height-10);  
    shell_cursor = &shell_cursor_struct;  
    init_fprint_global(framebuffer, shell_cursor, color);
    draw_boot_logo_lim(framebuffer, fb_height / 4, "EPXIUM", COLOR_NEON_GREEN, 30, 
                "This is EPXIUM, a homemade OS by EPAXGAMING", COLOR_NEON_GREEN);
    clear_screen_lim(framebuffer,COLOR_BLACK);
}

void init_mem(void){
   printOK("initing pmm ");
   pmm_init();
   printOK("init pmm sucessfull ");

   printOK("initing vmm ");
   vmm_init();
   printOK("init vmm sucessfull");
   
   uint64_t virt_base = 0xFFFF800010000000;
    void* phys_pages[3];
    uint64_t virt_addrs[3];

    for (int i = 0; i < 3; i++) {
        phys_pages[i] = pmm_alloc_page();
        if (phys_pages[i]) {
            virt_addrs[i] = virt_base + i * PAGE_SIZE;
            vmm_map_page(virt_addrs[i], (uint64_t)phys_pages[i], 0x3);
            *(uint64_t*)virt_addrs[i] = 0xDEADBEEF + i;
        }
    }

    for (int i = 0; i < 3; i++) {
        if (phys_pages[i]) {
            uint64_t value = *(uint64_t*)virt_addrs[i];
            vmm_unmap_page(virt_addrs[i]);
            pmm_free_page(phys_pages[i]);
        }
    }

    void* test_phys = pmm_alloc_page();
    if (test_phys) {
        uint64_t test_virt = virt_base + 4 * PAGE_SIZE;
        vmm_map_page(test_virt, (uint64_t)test_phys, 0x3);
        *(uint64_t*)test_virt = 0xCAFEBABE;
        uint64_t read_value = *(uint64_t*)test_virt;
        vmm_unmap_page(test_virt);
        pmm_free_page(test_phys);
    }
        
        
    printOK("RAM TEST WAS SUCCESFULL ");
}

void test_divide_by_zero(void) {
    int a = 10;
    int b = 0;
    int c = a / b;  // Diese Zeile wird die Division durch Null auslösen und den Handler aktivieren
}

void kmain(void) {
  
    fb_init();
    
    init_interrupts();
    //init_mem();

    int x = 1 / 0;
    
    
    while (1) {
        
    }
}