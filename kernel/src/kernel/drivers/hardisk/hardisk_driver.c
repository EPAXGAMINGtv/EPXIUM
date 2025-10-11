#include "hardisk_driver.h"
#include "kernel/io/io.h"  

void ahci_init(uintptr_t base_address) {
    outl(base_address + AHCI_GHC, AHCI_CMD_START);
    while (!(inl(base_address + AHCI_GHC) & AHCI_CMD_START));
}


void ahci_reset_port(uintptr_t port_base) {
    outl(port_base + AHCI_PORT_COMMAND, 0x01);  
    while (inl(port_base + AHCI_PORT_STATUS) & 0x01);  
}


void ahci_port_ready(uintptr_t port_base) {
    while (inl(port_base + AHCI_PORT_STATUS) & 0x01 == 0);  
}

void ahci_read(uintptr_t port_base, uint64_t sector, void* buffer) {
    ahci_port_ready(port_base);
    uint32_t* command = (uint32_t*)(port_base + AHCI_PORT_COMMAND);
    *command = AHCI_CMD_READ;
    outl(port_base + 0x10, (uint32_t)(sector & 0xFFFFFFFF));
    outl(port_base + 0x14, (uint32_t)((sector >> 32) & 0xFFFFFFFF));
    outl(port_base + AHCI_PORT_COMMAND, AHCI_CMD_READ);
    while (inl(port_base + AHCI_PORT_STATUS) & 0x01);  
}


void ahci_write(uintptr_t port_base, uint64_t sector, void* buffer) {
    ahci_port_ready(port_base);
    uint32_t* command = (uint32_t*)(port_base + AHCI_PORT_COMMAND);
    *command = AHCI_CMD_WRITE;
    outl(port_base + 0x10, (uint32_t)(sector & 0xFFFFFFFF));
    outl(port_base + 0x14, (uint32_t)((sector >> 32) & 0xFFFFFFFF));
    outl(port_base + AHCI_PORT_COMMAND, AHCI_CMD_WRITE);
    while (inl(port_base + AHCI_PORT_STATUS) & 0x01);  
}

int ahci_get_active_ports(uintptr_t base_address) {
    int active_ports = 0;
    for (int i = 0; i < MAX_PORTS; i++) {
        uintptr_t port_base = base_address + AHCI_PORTS + (i * 0x80); 
        if (inl(port_base + AHCI_PORT_STATUS) & 0x01) {
            active_ports++;
        }
    }

    return active_ports;
}