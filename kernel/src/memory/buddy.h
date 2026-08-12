// buddy.h
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <boot/limine.h>

#ifndef _BUDDY_H
#define _BUDDY_H

#define BUDDY_PAGE_SIZE 0x1000
#define BUDDY_MAX_PHYS ((uint64_t)1 * 1024 * 1024 * 1024)
#define BUDDY_MAX_PAGES (BUDDY_MAX_PHYS / BUDDY_PAGE_SIZE)
#define BUDDY_MAX_ORDER 18

#define BUDDY_INVALID_ADDRESS ((uintptr_t)-1)

void buddy_init(struct limine_memmap_response *response);
uintptr_t buddy_alloc_pages(size_t pages);
void buddy_free_pages(uintptr_t phys_addr, size_t pages);
uintptr_t buddy_alloc_page(void);
void buddy_free_page(uintptr_t phys_addr);

#endif