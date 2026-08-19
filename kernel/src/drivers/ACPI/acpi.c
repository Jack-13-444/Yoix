#include "drivers/ACPI/acpi.h"
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <memory/vmm.h>

int strncmp(const char* s1, const char* s2, size_t n)
{
    while (n > 0 && *s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
        n--;
    }

    if (n == 0)
        return 0;

    return (int)(unsigned char)(*s1) - (int)(unsigned char)(*s2);
}

bool doChecksum(struct ACPISDTHeader *tableHeader)
{
    unsigned char sum = 0;

    for (int i = 0; i < tableHeader->Length; i++)
    {
        sum += ((char *) tableHeader)[i];
    }

    return sum == 0;
}

struct XSDT *GetXSDT(void* phy_address)
{
    struct XSDP_t* xsdp = (struct XSDP_t*)phy_address;
    if (xsdp->Revision != 2 || xsdp->XsdtAddress == 0)
    {
        return NULL;
    }
    
    return (struct XSDT *)phy_to_virt(xsdp->XsdtAddress);  
        
}
struct RSDT *GetRSDT(void* phy_address)
{
    struct RSDP_t* rsdp = (struct RSDP_t*)phy_address;
    return (struct RSDT *)phy_to_virt(rsdp->RsdtAddress);  

}

void *findTable(void* phy_address, const char* Table_Sign)
{
    struct XSDT *xsdt = GetXSDT(phy_address);
    if (xsdt == NULL)
    {
        struct RSDT *rsdt = GetRSDT(phy_address);

        size_t entries_count = (rsdt->header.Length - sizeof(rsdt->header))/ 4;
        for (size_t i = 0; i < entries_count; i++)
        {
            struct ACPISDTHeader *h = (struct ACPISDTHeader *) phy_to_virt(rsdt->entries[i]);
            if (!strncmp(h->Signature, Table_Sign, 4))
            return (void *) h;
        }
    }else {

        size_t entries_count = (xsdt->h.Length - sizeof(xsdt->h))/ 8;
        for (size_t i = 0; i < entries_count; i++)
        {
            struct ACPISDTHeader *h = (struct ACPISDTHeader *) phy_to_virt(xsdt->PointerToOtherSDT[i]);
            if (!strncmp(h->Signature, Table_Sign, 4))
                return (void *) h;
        }
    }
    return NULL;

}
