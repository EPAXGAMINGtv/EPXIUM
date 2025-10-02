#include "kernel/io/io.h"  

void shutdown() {
    uint16_t pm1a_cnt_port = 0x604;  
    uint16_t pm1b_cnt_port = 0x605;  
    uint16_t pm2_cnt_port = 0xA00;   
    uint16_t pm3_cnt_port = 0xB00;   
    uint16_t slp_typ = 0x2000;        
    uint16_t slp_en  = 1 << 13;       
    outw(pm1a_cnt_port, slp_typ | slp_en);  
    outw(pm1b_cnt_port, slp_typ | slp_en);   
    outw(pm2_cnt_port, slp_typ | slp_en);   
    outw(pm3_cnt_port, slp_typ | slp_en);   
    for (;;) {
        __asm__ volatile("hlt");
    }
}
