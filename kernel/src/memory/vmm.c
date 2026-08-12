#include "memory/vmm.h"
#include <memory/buddy.h>


uint64_t phy_to_virt(uint64_t phys) {

    return phys + hhdm_request.response->offset;
}

void map_page() {
    // Make sure that both addresses are page-aligned.


    // Now you need to flush the entry in the TLB
    // or you might not notice the change.
}