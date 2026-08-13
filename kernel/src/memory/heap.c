// slub (not slab)
#include "memory/heap.h"
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <boot/limine.h>
#include <memory/buddy.h>

extern volatile struct limine_hhdm_request hhdm_request;

#define HEAP_MAGIC_SMALL 0x534C5542U
#define HEAP_MAGIC_LARGE 0x4C415247U
#define HEAP_MIN_OBJECT_SIZE 8U
#define HEAP_MAX_SMALL_SIZE 2048U
#define HEAP_CACHE_CLASS_COUNT 9

static void *memset(void *s, int c, size_t n) {
    uint8_t *p = s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}


static const size_t heap_cache_sizes[HEAP_CACHE_CLASS_COUNT] = {
    8U, 16U, 32U, 64U, 128U, 256U, 512U, 1024U, 2048U
};

typedef struct heap_slab_page {
    uint32_t magic;
    uint16_t object_size;
    uint16_t total_objects;
    uint16_t free_count;
    uint16_t reserved;
    uintptr_t free_list;
    uintptr_t next;
    uint64_t bitmap[256];

} heap_slab_page_t;

typedef struct heap_cache {
    uint16_t object_size;
    uintptr_t partial_pages;
} heap_cache_t;

typedef struct heap_large_header {
    uint32_t magic;
    uint32_t page_count;
} heap_large_header_t;

static heap_cache_t heap_caches[HEAP_CACHE_CLASS_COUNT];

static inline uint8_t *heap_phy_to_virt(uintptr_t phys) {
    if (hhdm_request.response == NULL) {
        return NULL;
    }

    return (uint8_t *)(phys + hhdm_request.response->offset);
}

static inline uintptr_t heap_virt_to_phy(void *virt) {
    if (hhdm_request.response == NULL) {
        return 0;
    }

    return (uintptr_t)virt - hhdm_request.response->offset;
}
static uint64_t Get_Bit(int64_t index)
{
    return index % 64 ;
}
static uint64_t Get_offset(int64_t index)
{
    return (uint64_t)1 << Get_Bit(index);
}
static uint64_t Get_array(int64_t index)
{
    return index / 64;
}

static int heap_cache_index(size_t size) {
    size_t rounded = size;

    if (rounded < HEAP_MIN_OBJECT_SIZE) {
        rounded = HEAP_MIN_OBJECT_SIZE;
    }

    for (int index = 0; index < HEAP_CACHE_CLASS_COUNT; index++) {
        if (rounded <= heap_cache_sizes[index]) {
            return index;
        }
    }

    return -1;
}
static int64_t object_index(uintptr_t ptr, uintptr_t virt_page, heap_slab_page_t* page)
{
    uint64_t object_buffer = virt_page + sizeof(heap_slab_page_t); 
    if(ptr < object_buffer) return -1;
     
    uint64_t offset = (uint64_t)(ptr - object_buffer);
    if ((offset % page->object_size) == 0) return -1;

    uint64_t end = offset / page->object_size;
    if (end >= page->total_objects  ) return -1;
    return end;
}


static void heap_remove_slab_from_cache(heap_cache_t *cache, uintptr_t slab_phys) {
    uintptr_t prev = 0;
    uintptr_t current = cache->partial_pages;

    while (current != 0) {
        heap_slab_page_t *page = (heap_slab_page_t *)heap_phy_to_virt(current);
        if (page == NULL) {
            return;
        }

        if (current == slab_phys) {
            if (prev == 0) {
                cache->partial_pages = page->next;
            } else {
                heap_slab_page_t *prev_page = (heap_slab_page_t *)heap_phy_to_virt(prev);
                if (prev_page == NULL) {
                    return;
                }
                prev_page->next = page->next;
            }
            page->next = 0;
            return;
        }

        prev = current;
        current = page->next;
    }
}

static void heap_insert_slab_into_cache(heap_cache_t *cache, uintptr_t slab_phys) {
    heap_slab_page_t *page = (heap_slab_page_t *)heap_phy_to_virt(slab_phys);
    if (page == NULL) {
        return;
    }

    page->next = cache->partial_pages;
    cache->partial_pages = slab_phys;
}

static uintptr_t heap_alloc_slab_page(uint16_t object_size) {
    uintptr_t phys = buddy_alloc_page();
    if (phys == BUDDY_INVALID_ADDRESS) {
        return BUDDY_INVALID_ADDRESS;
    }

    uint8_t *page_virt = heap_phy_to_virt(phys);
    if (page_virt == NULL) {
        buddy_free_page(phys);
        return BUDDY_INVALID_ADDRESS;
    }

    heap_slab_page_t *page = (heap_slab_page_t *)page_virt;
    page->magic = HEAP_MAGIC_SMALL;
    page->object_size = object_size;
    page->total_objects = (uint16_t)((BUDDY_PAGE_SIZE - sizeof(heap_slab_page_t)) / object_size);
    page->free_count = page->total_objects;
    page->reserved = 0;
    page->next = 0;
    memset(page->bitmap, 0, sizeof(page->bitmap));

    uint8_t *object_buffer = page_virt + sizeof(heap_slab_page_t);
    page->free_list = (uintptr_t)object_buffer;

    for (uint16_t index = 0; index < page->total_objects; index++) {
        uintptr_t *next_entry = (uintptr_t *)object_buffer;
        if (index + 1 < page->total_objects) {
            object_buffer += object_size;
            *next_entry = (uintptr_t)object_buffer;
        } else {
            *next_entry = 0;
        }
    }

    return phys;
}

static void *heap_alloc_from_cache(heap_cache_t *cache) {
    if (cache->partial_pages == 0) {
        uintptr_t slab = heap_alloc_slab_page(cache->object_size);
        if (slab == BUDDY_INVALID_ADDRESS) {
            return NULL;
        }
        cache->partial_pages = slab;
    }

    uintptr_t slab_phys = cache->partial_pages;
    heap_slab_page_t *page = (heap_slab_page_t *)heap_phy_to_virt(slab_phys);
    if (page == NULL || page->free_list == 0) return NULL;


    void *object = (void *)page->free_list;
    int64_t obj_index = object_index((uintptr_t)object, (uintptr_t)page, page); // (uintptr_t) is page_virt
    if (obj_index == -1) return NULL;

    uint64_t obj_masked =  Get_offset(obj_index);
    page->bitmap[Get_array(obj_index)] |= obj_masked;

    page->free_list = *(uintptr_t *)object;
    page->free_count--;

    

    if (page->free_count == 0) {
        heap_remove_slab_from_cache(cache, slab_phys);
    }

    return object;
}

static void heap_free_large( heap_large_header_t *header, uintptr_t page_phys) {
    if (header->magic != HEAP_MAGIC_LARGE) {
        return;
    }

    buddy_free_pages(page_phys, header->page_count);
}

static void heap_free_small(void *ptr, heap_slab_page_t *page, uintptr_t page_phys) {
    if (page->magic != HEAP_MAGIC_SMALL)
    {
        return;
    }
    uintptr_t page_virt = (uintptr_t)heap_phy_to_virt(page_phys);
    int64_t obj_index  = object_index((uintptr_t)ptr, page_virt, page);
    if (obj_index == -1)
    {
        return;
    }
    
    uint64_t obj_masked = Get_offset(obj_index);
    int index = heap_cache_index(page->object_size);

    if ((page->bitmap[Get_array(obj_index)] & obj_masked) == 0)
    {
        return;
    }

    if (index < 0) {
        return;
    }

    
    heap_cache_t *cache = &heap_caches[index];
    uintptr_t object = (uintptr_t)ptr;
    *(uintptr_t *)object = page->free_list;
    page->free_list = object;
    page->free_count++;
    page->bitmap[Get_array(obj_index)] &= ~obj_masked; 


    if (page->free_count == page->total_objects) {
        heap_remove_slab_from_cache(cache, page_phys);
        buddy_free_page(page_phys);
        return;
    }

    if (page->free_count == 1) {
        heap_insert_slab_into_cache(cache, page_phys);
    }
}

void heap_init(void) {
    for (int index = 0; index < HEAP_CACHE_CLASS_COUNT; index++) {
        heap_caches[index].object_size = (uint16_t)heap_cache_sizes[index];
        heap_caches[index].partial_pages = 0;
    }
}

void *kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    int cache_index = heap_cache_index(size);
    if (cache_index >= 0) {
        return heap_alloc_from_cache(&heap_caches[cache_index]);
    }

    size_t total_size = size + sizeof(heap_large_header_t);
    size_t pages = (total_size + BUDDY_PAGE_SIZE - 1) / BUDDY_PAGE_SIZE;
    uintptr_t phys = buddy_alloc_pages(pages);
    if (phys == BUDDY_INVALID_ADDRESS) {
        return NULL;
    }

    uint8_t *virt = heap_phy_to_virt(phys);
    if (virt == NULL) {
        buddy_free_pages(phys, pages);
        return NULL;
    }

    heap_large_header_t *header = (heap_large_header_t *)virt;
    header->magic = HEAP_MAGIC_LARGE;
    header->page_count = (uint32_t)pages;

    return virt + sizeof(heap_large_header_t);
}

void kfree(void *ptr) {
    if (ptr == NULL || hhdm_request.response == NULL) {
        return;
    }

    uintptr_t virt_addr = (uintptr_t)ptr;
    uintptr_t page_virt = virt_addr & ~(BUDDY_PAGE_SIZE - 1);
    uintptr_t page_phys = page_virt - hhdm_request.response->offset;
    uint8_t *page_start = heap_phy_to_virt(page_phys);
    if (page_start == NULL) {
        return;
    }

    heap_slab_page_t *slab_page = (heap_slab_page_t *)page_start;
    if (slab_page->magic == HEAP_MAGIC_SMALL) {
        heap_free_small(ptr, slab_page, page_phys);
        return;
    }

    heap_large_header_t *large_header = (heap_large_header_t *)page_start;
    if (large_header->magic == HEAP_MAGIC_LARGE) {
        heap_free_large(large_header, page_phys);
    }
}

