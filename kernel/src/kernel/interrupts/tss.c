#include "tss.h"
#include "../device/display/fprint.h"

tss_t tss;

void tss_init(void) {
    for (int i = 0; i < sizeof(tss_t); i++)
        ((uint8_t*)&tss)[i] = 0;

    tss.rsp0 = 0;       
    tss.io_map_base = sizeof(tss_t);

    fprint("TSS minimal init done\n");
}
