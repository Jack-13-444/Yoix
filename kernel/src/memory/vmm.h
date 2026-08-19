#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef _VMM_H
#define _VMM_H

#define PAGE_PRESENT   (1ULL << 0)
#define PAGE_WRITABLE  (1ULL << 1)
#define PAGE_USER      (1ULL << 2)
#define PAGE_ADDR_MASK 0x000FFFFFFFFFF000ULL  
#define PML4_INDEX(addr) (((addr) >> 39) & 0x1FF)
#define PDPT_INDEX(addr) (((addr) >> 30) & 0x1FF)
#define PD_INDEX(addr)   (((addr) >> 21) & 0x1FF)
#define PT_INDEX(addr)   (((addr) >> 12) & 0x1FF)
#define PAGE_PCD (1ULL << 4) 
#define PAGE_PWT (1ULL << 3)

uint64_t *phy_to_virt(uint64_t phys);


bool vmm_map(uintptr_t virt_addr, uintptr_t phys_addr, uint64_t flags);
bool vmm_unmap(uintptr_t virt_addr);
uintptr_t vmm_get_phys(uintptr_t virt_addr);




#endif
 