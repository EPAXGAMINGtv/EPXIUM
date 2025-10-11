#ifndef HARDISK_DRIVER_H
#define HARDISK_DRIVER_H

#include <stdint.h>

#define AHCI_BASE_ADDRESS 0x10000000  
#define AHCI_GHC 0x00     
#define AHCI_PORTS 0x20   

#define AHCI_CMD_START 0x01  
#define AHCI_PORT_STATUS 0x04
#define AHCI_PORT_COMMAND 0x08

#define AHCI_CMD_READ 0x01
#define AHCI_CMD_WRITE 0x02

#define MAX_PORTS 32

typedef struct {
    uint32_t command_list_base;
    uint32_t FIS_base;
    uint32_t interrupt_status;
    uint32_t command_issue;
} ahci_port_t;

void ahci_init(uintptr_t base_address);
void ahci_reset_port(uintptr_t port_base);
void ahci_read(uintptr_t port_base, uint64_t sector, void* buffer);
void ahci_write(uintptr_t port_base, uint64_t sector, void* buffer);
void ahci_port_ready(uintptr_t port_base);
int ahci_get_active_ports(uintptr_t base_address);
#endif