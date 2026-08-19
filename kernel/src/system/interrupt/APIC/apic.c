#include <stdbool.h>
#include <stdint.h>
#include <system/cpu/msr.h>
#include <system/cpu/cpuid.h>
#include <memory/vmm.h>


#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800
uintptr_t g_virt_apic;  
/** returns a 'true' value if the CPU supports APIC
 *  and if the local APIC hasn't been disabled in MSRs
 *  note that this requires CPUID to be supported.
 */
bool check_apic() {
   uint32_t eax, edx;
   cpuid(1, &eax, &edx);
   return edx & CPUID_FEAT_EDX_APIC;
}

/* Set the physical address for local APIC registers */
void cpu_set_apic_base(uintptr_t apic) {
   uint32_t edx = 0;
   uint32_t eax = (apic & 0xfffff000) | IA32_APIC_BASE_MSR_ENABLE;

   edx = (apic >> 32) & 0x0f;


   cpuSetMSR(IA32_APIC_BASE_MSR, eax, edx);
}

/**
 * Get the physical address of the APIC registers page
 * make sure you map it to virtual memory ;)
 */
uintptr_t cpu_get_apic_base() {
   uint32_t eax, edx;
   cpuGetMSR(IA32_APIC_BASE_MSR, &eax, &edx);

   return (eax & 0xfffff000) | ((edx & 0x0f) << 32);

}

void enable_apic() {
    /* Section 11.4.1 of 3rd volume of Intel SDM recommends mapping the base address page as strong uncacheable for correct APIC operation. */

    /* Hardware enable the Local APIC if it wasn't enabled */
    cpu_set_apic_base(cpu_get_apic_base());

    // /* Set the Spurious Interrupt Vector Register bit 8 to start receiving interrupts */

    uintptr_t apic_phys = cpu_get_apic_base();
    g_virt_apic = (uintptr_t)phy_to_virt(apic_phys);
    vmm_map(g_virt_apic, apic_phys, PAGE_PRESENT | PAGE_WRITABLE | PAGE_PCD);   

    uint32_t *svr = (uint32_t*)((uint8_t*)g_virt_apic + 0xF0);
    *svr = 0x100 | 255;
}
void disable_apic() {
    uintptr_t apic_phys = cpu_get_apic_base();
    uintptr_t apic_virt = (uintptr_t)phy_to_virt(apic_phys);
    vmm_map(apic_virt, apic_phys, PAGE_PRESENT | PAGE_WRITABLE | PAGE_PCD);   

    uint32_t *svr = (uint32_t*)((uint8_t*)apic_virt + 0xF0);
    *svr &= ~0x100;

}
void write_lapic_register(uintptr_t lapic_base, uint32_t offset, uint32_t val)
{
    *(volatile uint32_t*)(lapic_base + offset) = val;
}
uint32_t read_lapic_register(uintptr_t lapic_base, uint32_t offset)
{
   return *(volatile uint32_t*)(lapic_base + offset);
}
void APIC_EOI()
{
   write_lapic_register(g_virt_apic, 0xB0, 0);   
}
