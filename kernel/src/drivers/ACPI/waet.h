#include <stdint.h>
#include <drivers/ACPI/acpi.h>
#ifndef _WAET_H

#define _WAET_H

struct WAET
{
    struct ACPISDTHeader h;
    uint32_t Emulated_Device_Flags;
}__attribute__((packed));
#endif 