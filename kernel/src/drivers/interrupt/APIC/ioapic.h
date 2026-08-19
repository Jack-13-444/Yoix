#include <stdint.h>
#include <drivers/ACPI/madt.h>
#include <memory/vmm.h>
#define IOAPIC_ADDRESS 0xffffAC0000000000
#ifndef _IOAPIC_H
#define _IOAPIC_H
union RedirectionEntry{

struct 
{
    uint64_t vector                 : 8;
    uint64_t Delivery_Mode          : 3;
    uint64_t Destination_Mode       : 1;
    uint64_t Delivery_Status        : 1;
    uint64_t Pin_Polarity           : 1;
    uint64_t Remote_IRR             : 1;
    uint64_t Trigger_Mode           : 1;
    uint64_t Mask                   : 1;
    uint64_t Reserved               : 39;
    uint64_t destination            : 8;
    
};
struct 
{
    uint32_t lowerDword;
    uint32_t upperDword;
};
}__attribute__((packed));



uint32_t FindEntries(void* madt_header, uint32_t type, void** array, uint32_t max_count);
void write_ioapic_register(const uintptr_t apic_base, const uint8_t offset, const uint32_t val);
uint32_t read_ioapic_register(const uintptr_t apic_base, const uint8_t offset);
void init_IOAPIC(void* madt_header);
uint32_t ioapic_read(unsigned char regOff, uint8_t index);
void ioapic_write(unsigned char regOff, uint32_t data, uint8_t index);
void test();


#endif