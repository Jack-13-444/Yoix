#include <stdint.h>
#include <drivers/ACPI/acpi.h>

struct MCFG_Entry
{
    uint64_t base_address;  // Base address of enhanced configuration mechanism
    uint16_t PCI_segment;   // PCI Segment Group Number
    uint8_t  PCI_start;     // Start PCI bus number decoded by this host bridge
    uint8_t  PCI_end;       // End PCI bus number decoded by this host bridge
    uint32_t Reserved2;
}__attribute__((packed));

struct MCFG
{
    struct ACPISDTHeader h;
    uint64_t Reserved1;
    struct MCFG_Entry entries[] ;
}__attribute__((packed));
