#include <stdint.h>
#include <drivers/ACPI/acpi.h>
struct WAET
{
    struct ACPISDTHeader h;
    uint32_t Emulated_Device_Flags;
}__attribute__((packed));
