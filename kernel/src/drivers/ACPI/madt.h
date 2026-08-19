#include <stdint.h>
#include <drivers/ACPI/acpi.h>
#ifndef _MADT_H
#define _MADT_H

struct MADT_t
{
    struct ACPISDTHeader h;
    uint32_t Local_APIC_Address;
    uint32_t Flags;
}__attribute__ ((packed));


struct PLA //  Processor Local APIC
{
    uint8_t     Entry_type;
    uint8_t     Record_Length;
    uint8_t     ACPI_Processor_ID;
    uint8_t     APIC_ID;
    uint32_t    Flags;
}__attribute__((packed));

struct IOAPIC
{
    uint8_t     Entry_type;
    uint8_t     Record_Length;
    uint8_t     IOAPIC_ID;
    uint8_t     Reserved;
    uint32_t    IOAPIC_Address;
    uint32_t    GSI_base; // Global System Interrupt Base   

}__attribute__((packed));

struct IOAPIC_ISO // Interrupt Source Override
{
    uint8_t     Entry_type;
    uint8_t     Record_Length;
    uint8_t     Bus_Source;
    uint8_t     IRQ_Source;
    uint32_t    GSI; // Global System Interrupt for Interrupt Source Override 
    uint16_t    Flags;
}__attribute__((packed));

struct IOAPIC_NMI // Non-maskable interrupt source
{
    uint8_t     Entry_type;
    uint8_t     Record_Length;
    uint8_t     NMI_Source;
    uint8_t     Reserved;
    uint16_t    Flags;
    uint32_t    GSI; // Global System Interrupt for Non-maskable interrupt source
}__attribute__((packed));

struct LAPIC_NMI // Non-maskable interrupt source FOR LOCAL apic
{
    uint8_t     Entry_type;
    uint8_t     Record_Length;
    uint8_t     ACPI_Processor_ID;
    uint16_t    Flags;
    uint8_t     LINT;
}__attribute__((packed));

struct LAPIC_AO // Local APIC Address Override
{
    uint8_t     Entry_type;
    uint8_t     Record_Length;
    uint16_t     Reserved;
    uint64_t    LAPIC_Address; // 64-bit physical address of Local APIC

}__attribute__((packed));

struct LAPIC_X2 // : Processor Local x2APIC
{
    uint8_t     Entry_type      ;
    uint8_t     Record_Length   ;
    uint16_t    Reserved       ;
    uint32_t    x2LAPIC         ;// Processor's local x2APIC ID
    uint32_t    FLAGS           ;
    uint32_t    ACPI_ID         ;
}__attribute__((packed));
#endif