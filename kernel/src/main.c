#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <boot/limine.h>
#include <memory/gdt.h>
#include <memory/buddy.h>
#include <memory/vmm.h>
#include <memory/heap.h>
#include <system/interrupt/idt.h>
#include <system/interrupt/APIC/apic.h>
#include <system/interrupt/PIC/pic.h>
#include <system/cpu/msr.h>
#include <drivers/ACPI/acpi.h>
#include <drivers/interrupt/APIC/ioapic.h>
#include <drivers/ACPI/madt.h>

#define KASSERT(cond) do { \
    if (!(cond)) { \
        for (;;) { \
            asm volatile("cli; hlt"); \
        } \
    } \
} while (0)
// Set the base revision to 6, this is recommended as this is the latest
// base revision described by the Limine boot protocol specification.
// See specification for further info.

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(6);

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent, _and_ they should be accessed at least
// once or marked as used with the "used" attribute as done here.

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0
};
__attribute__((used, section(".limine_requests")))
volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST_ID,
    .revision = 0
};
__attribute__((used, section(".limine_requests")))
volatile struct limine_rsdp_request RSDP_request = 
{
    .id = LIMINE_RSDP_REQUEST_ID,
    .revision = 2
};




// Finally, define the start and end markers for the Limine requests.
// These can also be moved anywhere, to any .c file, as seen fit.

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

// GCC and Clang reserve the right to generate calls to the following
// 4 functions even if they are not directly called.
// Implement them as the C specification mandates.
// DO NOT remove or rename these functions, or stuff will eventually break!
// They CAN be moved to a different .c file.

void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    uint8_t *restrict pdest = dest;
    const uint8_t *restrict psrc = src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = dest;
    const uint8_t *psrc = src;

    if ((uintptr_t)src > (uintptr_t)dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if ((uintptr_t)src < (uintptr_t)dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = s1;
    const uint8_t *p2 = s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

// Halt and catch fire function.
static void hcf(void) {
    for (;;) {
        asm ("hlt");
    }
}

static inline void trigger_gp(void) {
    uint16_t bad_selector = 0xFFFF; 
    __asm__ volatile ("mov %0, %%ds" : : "r"(bad_selector));
}

// The following will be our kernel's entry point.
// If renaming kmain() to something else, make sure to change the
// linker script accordingly.
void kmain(void) {
    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }
    if (memmap_request.response == NULL || memmap_request.response ->entry_count == 0 || memmap_request.response->entries == NULL ) {
        hcf();
    }
    if (hhdm_request.response == NULL)
    {
       hcf();
    }
    if (RSDP_request.response == NULL)
    {
        hcf();
    }
    

    // Fetch the first framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

    // Print a nice pattern to screen as an example.
    // Note: we assume the framebuffer model is RGB with 32-bit pixels.
    volatile uint32_t *fb_ptr = framebuffer->address;
    for (size_t y = 0; y < framebuffer->height; y++) {
        for (size_t x = 0; x < framebuffer->width; x++) {
            uint32_t nX = x * 255 / framebuffer->width;
            uint32_t nY = y * 255 / framebuffer->height;
            fb_ptr[y * (framebuffer->pitch / 4) + x] = (nY << 8) | nX;
        }
    }
    
    initGdt();
    idt_init();
    buddy_init(memmap_request.response);
    heap_init();
    enable_apic();

    void* RSDP_ADDRESS = RSDP_request.response->address;
    void*  MADT_Header = findTable(RSDP_ADDRESS, "APIC");  
    init_IOAPIC(MADT_Header);
    // void *small_ptr = kmalloc(16);
    // KASSERT(small_ptr != NULL);
    // uint32_t *small_data = (uint32_t *)small_ptr;
    // small_data[0] = 0xDEADBEEF;
    // KASSERT(small_data[0] == 0xDEADBEEF);

    // void *large_ptr = kmalloc(5000);
    // KASSERT(large_ptr != NULL);
    // uint8_t *large_data = (uint8_t *)large_ptr;
    // large_data[0] = 0xAA;
    // large_data[4095] = 0x55;
    // KASSERT(large_data[0] == 0xAA);
    // KASSERT(large_data[4095] == 0x55);

    // kfree(small_ptr);
    // kfree(large_ptr);

    // void *reuse_ptr = kmalloc(16);
    // KASSERT(reuse_ptr != NULL);
    // kfree(reuse_ptr);

    // uintptr_t a = buddy_alloc_page();
    // KASSERT(a != BUDDY_INVALID_ADDRESS);

    // uintptr_t b = buddy_alloc_pages(4);
    // KASSERT(b != BUDDY_INVALID_ADDRESS);

    // buddy_free_page(a);
    // uintptr_t c = buddy_alloc_page();
    // KASSERT(c == a);   

    // buddy_free_page(c);
    // // buddy_free_pages(b, 4);
    // uintptr_t phys = buddy_alloc_page();
    // uintptr_t virt = 0xffff900000000000; 
    // bool ok = vmm_map(virt, phys, PAGE_PRESENT | PAGE_WRITABLE);
    // uint64_t result = vmm_get_phys(virt + 0x50);
    // bool unmap_ok = vmm_unmap(virt);

    // We're done, just hang...

    hcf();
}