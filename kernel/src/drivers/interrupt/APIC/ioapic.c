#include <drivers/interrupt/APIC/ioapic.h>
#include <memory/buddy.h>
#include <system/interrupt/APIC/apic.h>

// Global arrays
struct IOAPIC       *ioapic_entries[16];
struct IOAPIC_ISO   *iso_entries[256];
uintptr_t phys_address[16]; // ioapic phys addresses
uintptr_t virt_Address[16]; // ioapic virt addresses
uint32_t ioapic_count = 0;
uint32_t iso_count = 0;
uint32_t IOAPICVER = 0;
uint32_t IOAPICID  = 0;

extern uintptr_t g_virt_apic;
uint32_t FindEntries(void* madt_header, uint32_t type, void** array, uint32_t max_count)
{
    struct MADT_t* madt = (struct MADT_t*)madt_header;
    uint8_t* entries_end = (uint8_t*)madt + madt->h.Length;
    uint8_t* ptr = (uint8_t*)madt + sizeof(struct MADT_t);
    uint32_t count = 0;
    
    while (ptr < entries_end)
    {
        uint8_t entry_type = *ptr;
        uint8_t entry_length = *(ptr + 1);
        
        if (entry_type == type && array != NULL && count < max_count)
        {
             array[count] = ptr;
             count++;
        }
        
        if (entry_length == 0) break;
        ptr += entry_length;
    }
    
    return count;
}


void init_IOAPIC(void* madt_header)
{
    void* ioapic_ptr[16];
    void* iso_ptr[256];
    ioapic_count = FindEntries(madt_header, 1, ioapic_ptr, 16);
    for (size_t i = 0; i < ioapic_count; i++)
    {
        ioapic_entries[i] = (struct IOAPIC *)ioapic_ptr[i];
    }
    iso_count = FindEntries(madt_header, 2, iso_ptr, 256);
    for (size_t i = 0; i < iso_count; i++)
    {
        iso_entries[i] = (struct IOAPIC_ISO *)iso_ptr[i];

    }
    // put the phys addresses
    for (size_t i = 0; i < ioapic_count; i++)
    {
        phys_address[i] = ioapic_entries[i]->IOAPIC_Address;   
    }
    // make virt addresses
    for (size_t i = 0; i < ioapic_count; i++)
    {
        virt_Address[i]     = phy_to_virt(buddy_alloc_page());
        vmm_map(virt_Address[i], phys_address[i], PAGE_PRESENT | PAGE_WRITABLE | PAGE_PCD);

    }
    IOAPICVER = ioapic_read(0x01, 0);
    IOAPICID  = ioapic_read(0x00, 0);    
}
uint32_t Get_iso_gsi(uint8_t irq)
{
    for (size_t i = 0; i < iso_count; i++)
    {
        if (iso_entries[i]->IRQ_Source == irq )
        {
            return iso_entries[i]->GSI;
        }
        
    }
    return irq;
}
uint32_t ioapic_read(unsigned char regOff, uint8_t index)
{
    *(uint32_t volatile*) virt_Address[index] = regOff;
    return *(uint32_t volatile*)(virt_Address[index] + 0x10);
}

        /*
         * Writes the data into the register associated. 
         *
         * @param regOff - the register's offset which is being written
         * @param data - dword to write to the register
         */
void ioapic_write(unsigned char regOff, uint32_t data, uint8_t index)
{
        *(uint32_t volatile*) virt_Address[index] = regOff;
        *(uint32_t volatile*)(virt_Address[index] + 0x10) = data;
}
uint8_t OffsetLower(uint8_t pin)
{
    return (0x10 + (pin * 2));
}
uint8_t OffsetUpper(uint8_t pin)
{
    return (0x10 + (pin * 2)+1);
}
uint8_t Get_pin(struct IOAPIC* ioapic_entry, uint8_t irq)
{
    return (Get_iso_gsi(irq) - ioapic_entry->GSI_base);
}

uint8_t Get_APICID()
{
    uint8_t lapic_id = read_lapic_register(g_virt_apic, 0x20);
    return (lapic_id >> 24);
}
void writeRedir(union RedirectionEntry *Redir, uint8_t vector, uint8_t delv_mode, uint8_t dest_mode, uint8_t Pin_Polarity, uint8_t Trigger_mode, uint8_t mask, uint8_t dest)
{
    Redir->vector            = vector;
    Redir->Delivery_Mode     = delv_mode;
    Redir->Destination_Mode  = dest_mode;
    Redir->Pin_Polarity      = Pin_Polarity;
    Redir->Trigger_Mode      = Trigger_mode;
    Redir->Mask              = mask;
    Redir->destination       = dest;
}
void loadRedir(union RedirectionEntry *Redir, struct IOAPIC* ioapic_entry, uint8_t irq)
{
    uint8_t pin = Get_pin(ioapic_entry, irq);
    ioapic_write(OffsetLower(pin),Redir->lowerDword,0);
    ioapic_write(OffsetUpper(pin),Redir->upperDword,0);
}