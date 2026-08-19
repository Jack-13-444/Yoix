#include <stdint.h>
#include <drivers/ACPI/acpi.h>
#ifndef _SRAT_H
#define _SRAT_H

struct SRAT
{
    struct ACPISDTHeader h;
    uint8_t reserved[12];
} __attribute__((packed));

struct SRAT_proc_lapic_struct
{
    uint8_t type;      // 0x0 for this type of structure
    uint8_t length;    // 16
    uint8_t lo_DM;     // Bits [0:7] of the proximity domain
    uint8_t APIC_ID;   // Processor's APIC ID
    uint32_t flags;    // Flags (see below)
    uint8_t SAPIC_EID; // The processor's local SAPIC EID.
    uint8_t hi_DM[3];  // Bits [8:31] of the proximity domain
    uint32_t _CDM;     // The clock domain which the processor belongs to (more jargon)
} __attribute__((packed));
struct SRAT_mem_struct
{
    uint8_t type;         // 0x1 for this type of structure
    uint8_t length;       // 40
    uint32_t domain;      // The domain to which this memory region belongs to
    uint8_t reserved1[2]; // Reserved
    uint32_t lo_base;     // Low 32 bits of the base address of the memory range
    uint32_t hi_base;     // High 32 bits of the base address of the memory range
    uint32_t lo_length;   // Low 32 bits of the length of the range
    uint32_t hi_length;   // High 32 bits of the length
    uint8_t reserved2[4]; // Reserved
    uint32_t flags;       // Flags (see below)
    uint8_t reserved3[8]; // Reserved
} __attribute__ ((packed));

struct SRAT_proc_lapic2_struct
{
    uint8_t type;         // 0x2 for this type of structure
    uint8_t length;       // 24
    uint8_t reserved1[2]; // Must be zero
    uint32_t domain;      // The proximity domain which the logical processor belongs to
    uint32_t x2APIC_ID;   // Processor's x2APIC ID
    uint32_t flags;
    uint32_t _CDM;        // The clock domain which the processor belongs to (more jargon)
    uint8_t reserved2[4]; // Reserved.
} __attribute__((packed));
#endif 