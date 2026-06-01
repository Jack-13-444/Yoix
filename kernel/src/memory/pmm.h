#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <boot/limine.h>

#ifndef _PMM_H
#define _PMM_H

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))
    
// buddy system
struct Free_List
{
    struct Free_List *prev;
    struct Free_List *next;
};

struct Buddy
{
    struct Free_List free_list[12];
    uint64_t Current_Order;
};

struct block
{
    uint64_t address;
    uint64_t size;
    bool IsFree;
    struct Free_List node;
};
void memory_map(struct limine_memmap_response *MMap, uint64_t order ,struct block *blocks);

void init_buddy(struct limine_memmap_response *MMap);
void split(struct Buddy *Buddy, struct block *block, uint64_t order);
void* kmalloc(size_t size);
void* kfree(void *ptr);

#endif