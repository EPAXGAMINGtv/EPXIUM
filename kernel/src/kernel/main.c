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
#include "kernel/mem/umalloc.h"
#include "kernel/mem/kmalloc.h"
#include "kernel/interrupts/drivers/keyboard.h"
#include "kernel/kernel_lib/random.h"
#include "kernel/interrupts/tss.h"
#include "kernel/processes/process.h"


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

__attribute__((used, section(".limine_requests")))
volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
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

void test_calc(){
    uint32_t num1 = random_u32();
    uint32_t num2 = random_u32();
    fprint_uint(num1);
    fprint(" + ");
    fprint_uint(num2);
    uint32_t end = num1+num2;
    fprint(" = ");
    fprint_uint(end);
      fprint("\n");
}


void print_logo(){
    fprintcolor("EEEEE   PPPPP   XX    XX    IIIIII     UU    UU   MM   MM\n",COLOR_GREEN);
    fprintcolor("EE      PP  PP    XX XX       II       UU    UU   MMM MMM\n",COLOR_GREEN);
    fprintcolor("EEEE    PPPPP      XX         II       UU    UU   MM M MM\n",COLOR_GREEN);
    fprintcolor("EE      PP        XX XX       II       UU    UU   MM   MM\n",COLOR_GREEN);
    fprintcolor("EEEEE   PP      XX    XX    IIIIII       UUUUU    MM   MM\n",COLOR_GREEN);
    fprintcolor("---------------------------------------------------------\n",COLOR_GREEN);
    fprint("\n");
    fprintcolor("---------------------------------------------------------\n",COLOR_GREEN);
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

    printOK("init irq ");
    irq_init();
    printOK("irq initzialized ");

    printOK("init timer");
    timer_init();
    printOK("timer initzialized");
    
    printOK("init isr ");
    isr_init();
    printOK("isr initzialized ");

    printOK("initing TSS");
    tss_init();
    printOK("tss init completet! ");

    printOK("init keyboard driver ");
    printOK("init keyboard interrupt ");
    printOK("clearing keyboard buffer ");
    init_keyboard();
    printOK("keyboard initzialized ");

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
    printOK("initing mem ");
    struct limine_memmap_response *memmap = memmap_request.response;
    vmm_init(memmap);
}



void init_kernel_lib(){
    printOK("init random"); 
    fprint("with: ");
    fprint_uint((uint32_t)get_tick_count());
    fprint(" ticks \n");
    random_init(get_tick_count());
    printOK("random init completetet ");
    fprint("during 500 test calculations in 5 seconds\n");

    for (int i = 5; i >0; i--) {
        sleep_s(1);  
        fprint_uint((uint32_t)i);  
        fprint("\n");
    }

    for (int i = 0; i < 500; i++) {
        test_calc();
    }

} 



void kmalloc_test(){
    framebuffer = framebuffer_request.response->framebuffers[0];  
    fb_width = framebuffer->width;
    fb_height = framebuffer->height;

    init_cursor(&shell_cursor_struct, fb_width-2, fb_height-10);  
    shell_cursor = &shell_cursor_struct;  

    void *virt = vmm_alloc_page();
    if (virt) fprint("virtuel page got test alloc!\n");
    void *phys = pmm_alloc_page();
    if (phys) fprint("pmm page got test allocated!\n");

    vmm_map_page(virt, phys,0x3);
    fprint("virtuel page got mapped!\n");

    vmm_free_page(virt);
    pmm_free_page(phys);
    fprint("all pages got freed!\n");
    kmalloc_init();

    void *a = kmalloc(512);
    void *b = kmalloc(2048);
    void *c = kmalloc(8192);

    fprint("Allocated 3 blocks from kernel heap\n");

    sleep_s(1);
    sleep_s(1);
    sleep_s(1);

    clear_screen_lim(framebuffer,COLOR_BLACK);
}


void kernel_init(void){
    fb_init();
    printOK("starting kernel init ");
    init_interrupts();
    init_mem();

    int x = 1 / 0;
    switch_layout(LAYOUT_DE);

    //unessential init
    init_kernel_lib();

    
    sleep_s(1);
    fprint("\n");
    printOK("kernel init completet ");
    sleep_s(5);
    clear_screen_lim(framebuffer,COLOR_DARK_GRAY);
    kmalloc_test();

    framebuffer = framebuffer_request.response->framebuffers[0];  
    fb_width = framebuffer->width;
    fb_height = framebuffer->height;

    init_cursor(&shell_cursor_struct, fb_width-2, fb_height-10);  
    shell_cursor = &shell_cursor_struct;  
    print_logo();
}


void kmain(void) {
    
    kernel_init();
   // userspace_main();
    
    while (1) {
       /*if(key_is_pressed(0x1C)){
            fprint("\nA pressed \n");
       } 
        */
      
       __asm__ volatile ("pause");
    }
}