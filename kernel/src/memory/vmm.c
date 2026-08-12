#include "memory/vmm.h"
#include <memory/buddy.h>
#include <boot/limine.h>

#define PAGE_PRESENT   (1ULL << 0)
#define PAGE_WRITABLE  (1ULL << 1)
#define PAGE_USER      (1ULL << 2)
#define PAGE_ADDR_MASK 0x000FFFFFFFFFF000ULL  
#define PML4_INDEX(addr) (((addr) >> 39) & 0x1FF)
#define PDPT_INDEX(addr) (((addr) >> 30) & 0x1FF)
#define PD_INDEX(addr)   (((addr) >> 21) & 0x1FF)
#define PT_INDEX(addr)   (((addr) >> 12) & 0x1FF)

extern volatile struct limine_hhdm_request hhdm_request;

uint64_t *phy_to_virt(uint64_t phys) {
    if (hhdm_request.response == NULL)
    {
        return NULL;
    }
    
    return (uint64_t *)(phys + hhdm_request.response->offset);
}

static inline uint64_t read_cr3(void) {
    uint64_t value;
    asm volatile("mov %%cr3, %0" : "=r"(value));
    return value;
}
static inline void invlpg_tlb(unsigned long addr) {
   asm volatile("invlpg (%0)" ::"r" (addr) : "memory");
}

uintptr_t Make_Page() {
    // Create a new page
    uintptr_t page_phy = buddy_alloc_page();
    if (page_phy == BUDDY_INVALID_ADDRESS)
    {
        return BUDDY_INVALID_ADDRESS;
    }
    uint64_t *page_virt = (uint64_t *)phy_to_virt(page_phy);
    for (int i = 0; i < 512; i++)
    {
        page_virt[i] = 0;
    }

    return page_phy;

}
uint64_t* vmm_get_next_level(uint64_t* table, uint64_t index)
{
    uint64_t entry = table[index];
    uint64_t next_phys ;
    if ((entry & PAGE_PRESENT) == 0)
    {
        uintptr_t new_table_phys = Make_Page(); 
        if (new_table_phys == BUDDY_INVALID_ADDRESS)
        {
            return NULL;
        }
        
        table[index] = new_table_phys | PAGE_PRESENT | PAGE_WRITABLE;
        next_phys = (uint64_t)new_table_phys;        
    }else
    {
        next_phys = (uint64_t)(entry & PAGE_ADDR_MASK);
    }

    return phy_to_virt(next_phys);
    
}
uint64_t* vmm_get_existing_level(uint64_t* table, uint64_t index)
{
    uint64_t entry = table[index];
    uint64_t next_phys ;
    if ((entry & PAGE_PRESENT) == 0)
    {
        return NULL;  
    }

    next_phys = (uint64_t)(entry & PAGE_ADDR_MASK);
    
    return phy_to_virt(next_phys);
}    

bool vmm_map(uintptr_t virt_addr, uintptr_t phys_addr, uint64_t flags)
{
    uint64_t pml4_index = PML4_INDEX(virt_addr);
    uint64_t pdpt_index = PDPT_INDEX(virt_addr);
    uint64_t pd_index   = PD_INDEX(virt_addr);
    uint64_t pt_index   = PT_INDEX(virt_addr);

    uint64_t* pml4 = phy_to_virt(read_cr3() & PAGE_ADDR_MASK);
    if (pml4 == NULL) {

        return false;
    }
    uint64_t* pdpt = vmm_get_next_level(pml4, pml4_index);
    if (pdpt == NULL) {
        return false;
    }
    uint64_t* pd   = vmm_get_next_level(pdpt, pdpt_index);
    if (pd == NULL) {
        return false;
    }
    uint64_t* pt   = vmm_get_next_level(pd, pd_index);
    if (pt == NULL) {

        return false;
    }
    pt[pt_index] = (phys_addr & PAGE_ADDR_MASK) | flags;
    invlpg_tlb(virt_addr);

    return true;

}

bool vmm_unmap(uintptr_t virt_addr)
{

    uint64_t pml4_index = PML4_INDEX(virt_addr);
    uint64_t pdpt_index = PDPT_INDEX(virt_addr);
    uint64_t pd_index   = PD_INDEX(virt_addr);
    uint64_t pt_index   = PT_INDEX(virt_addr);

    uint64_t* pml4 = phy_to_virt(read_cr3() & PAGE_ADDR_MASK);
    if (pml4 == NULL) {
        return false;
    }
    uint64_t* pdpt = vmm_get_existing_level(pml4, pml4_index);
    if (pdpt == NULL) {
        return false;
    }
    uint64_t* pd   = vmm_get_existing_level(pdpt, pdpt_index);
    if (pd == NULL) {
        return false;
    }
    uint64_t* pt   = vmm_get_existing_level(pd, pd_index);
    if (pt == NULL) {
        return false;
    }
    uint64_t old_entry = pt[pt_index];

    if ((old_entry & PAGE_PRESENT) == 0) 
    {
        return false;
    }
    
    pt[pt_index] = 0;    
    invlpg_tlb(virt_addr);

    buddy_free_page(old_entry & PAGE_ADDR_MASK);


    return true;

}

uintptr_t vmm_get_phys(uintptr_t virt_addr)
{

    uint64_t pml4_index = PML4_INDEX(virt_addr);
    uint64_t pdpt_index = PDPT_INDEX(virt_addr);
    uint64_t pd_index   = PD_INDEX(virt_addr);
    uint64_t pt_index   = PT_INDEX(virt_addr);

    uint64_t* pml4 = phy_to_virt(read_cr3() & PAGE_ADDR_MASK);
    if (pml4 == NULL) {
        return BUDDY_INVALID_ADDRESS;
    }
    uint64_t* pdpt = vmm_get_existing_level(pml4, pml4_index);
    if (pdpt == NULL) {
        return BUDDY_INVALID_ADDRESS;
    }
    uint64_t* pd   = vmm_get_existing_level(pdpt, pdpt_index);
    if (pd == NULL) {
        return BUDDY_INVALID_ADDRESS;
    }
    uint64_t* pt   = vmm_get_existing_level(pd, pd_index);
    if (pt == NULL) {
        return BUDDY_INVALID_ADDRESS;
    }
    uint64_t entry = pt[pt_index];

    if ((entry & PAGE_PRESENT) == 0) 
    {
        return BUDDY_INVALID_ADDRESS;
    }

    uint64_t phys_base = entry & PAGE_ADDR_MASK;
    uint64_t offset = virt_addr & 0xFFF;
    uintptr_t phys_addr = phys_base | offset;


    return phys_addr;

}
