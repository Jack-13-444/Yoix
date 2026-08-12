#include <memory/buddy.h>
// buddy.c
/* Buddy allocator page states.
 * - FREE_BODY marks pages that are part of a free block but are not the block head.
 * - ALLOCATED marks pages that are currently in use.
 * - RESERVED marks pages that are not part of the allocator's free pools.
 */
#define BUDDY_PAGE_STATE_FREE_BODY 0xFD
#define BUDDY_PAGE_STATE_ALLOCATED 0xFE
#define BUDDY_PAGE_STATE_RESERVED 0xFF
#define UNSPLITTABLE_SIZE 4096

/* Per-page state table for the allocator. Each entry is either an order
 * value for the start of a free block, FREE_BODY for continuation pages, or
 * ALLOCATED/RESERVED for non-free pages.
 */
static uint8_t buddy_page_state[BUDDY_MAX_PAGES];

/* Free list heads indexed by block order. Each entry points to the first frame
 * in a doubly-linked list of free blocks of that order.
 */
static uint32_t buddy_free_list[BUDDY_MAX_ORDER + 1];

/* Doubly-linked list pointers for free blocks. Only the head frame of a free
 * block is stored in the free lists, but we keep next/prev pointers for fast
 * removal.
 */

static uint32_t buddy_free_next[BUDDY_MAX_PAGES];
static uint32_t buddy_free_prev[BUDDY_MAX_PAGES];

static inline uint64_t buddy_align_up(uint64_t value, uint64_t alignment) {
    /* Round `value` up to the next multiple of `alignment`. */
    return (value + alignment - 1) & ~(alignment - 1);
}

static inline uint64_t buddy_align_down(uint64_t value, uint64_t alignment) {
    /* Round `value` down to the previous multiple of `alignment`. */
    return value & ~(alignment - 1);
}

static inline uint8_t buddy_order_from_pages(uint64_t pages) {
    /* Determine the smallest order such that 2^order pages can hold `pages`. */
    uint8_t order = 0;
    uint64_t size = 1ULL;

    while (size < pages && order < BUDDY_MAX_ORDER) {
        size <<= 1;
        order++;
    }

    return order;
}

static void buddy_reset(void) {
    /* Initialize the page-state and free-list arrays.
     * Mark everything reserved until usable ranges are added.
     */
    for (uint32_t i = 0; i < BUDDY_MAX_PAGES; i++) {
        buddy_page_state[i] = BUDDY_PAGE_STATE_RESERVED;
        buddy_free_next[i] = UINT32_MAX;
        buddy_free_prev[i] = UINT32_MAX;
    }

    for (uint32_t order = 0; order <= BUDDY_MAX_ORDER; order++) {
        buddy_free_list[order] = UINT32_MAX;
    }
}

static void buddy_mark_block_free(uint32_t frame, uint8_t order) {
    /* Mark the block starting at `frame` with the given order as free.
     * The head stores the order and the remainder stores FREE_BODY.
     */
    buddy_page_state[frame] = order;
    uint32_t block_size = 1U << order;

    for (uint32_t index = frame + 1; index < frame + block_size; index++) {
        buddy_page_state[index] = BUDDY_PAGE_STATE_FREE_BODY;
    }
}

static void buddy_mark_block_allocated(uint32_t frame, uint8_t order) {
    /* Mark every page in the block as allocated. */
    uint32_t block_size = 1U << order;

    for (uint32_t index = frame; index < frame + block_size; index++) {
        buddy_page_state[index] = BUDDY_PAGE_STATE_ALLOCATED;
    }
}

static void buddy_add_free_block(uint32_t frame, uint8_t order) {
    /* Add a free block to the list for its order. */
    buddy_mark_block_free(frame, order);

    if (buddy_free_list[order] != UINT32_MAX) {
        buddy_free_prev[buddy_free_list[order]] = frame;
    }

    buddy_free_prev[frame] = UINT32_MAX;
    buddy_free_next[frame] = buddy_free_list[order];
    buddy_free_list[order] = frame;
}

static bool buddy_remove_free_block(uint32_t frame, uint8_t order) {
    /* Remove a free block from its order list. Return true if successfully
     * removed, false if the block was not found or the list is inconsistent.
     */
    uint32_t previous = buddy_free_prev[frame];
    uint32_t next = buddy_free_next[frame];

    if (previous == UINT32_MAX) {
        if (buddy_free_list[order] != frame) {
            return false;
        }
        buddy_free_list[order] = next;
    } else if (buddy_free_next[previous] != frame) {
        return false;
    } else {
        buddy_free_next[previous] = next;
    }

    if (next != UINT32_MAX) {
        buddy_free_prev[next] = previous;
    }

    buddy_free_next[frame] = UINT32_MAX;
    buddy_free_prev[frame] = UINT32_MAX;
    return true;
}

static uint32_t buddy_pop_free_block(uint8_t order) {
    /* Pop the first free block from the list of the requested order. */
    uint32_t frame = buddy_free_list[order];

    if (frame == UINT32_MAX) {
        return UINT32_MAX;
    }

    buddy_free_list[order] = buddy_free_next[frame];

    if (buddy_free_list[order] != UINT32_MAX) {
        buddy_free_prev[buddy_free_list[order]] = UINT32_MAX;
    }

    buddy_free_next[frame] = UINT32_MAX;
    buddy_free_prev[frame] = UINT32_MAX;
    return frame;
}

static void buddy_add_range(uint32_t frame, uint32_t pages) {
    /* Split the range into aligned buddy blocks and add each free block. */
    while (pages > 0 ) {
        uint8_t order = BUDDY_MAX_ORDER;

        while ((1U << order) > pages || (frame & ((1U << order) - 1)) != 0) {
            order--;
        }

        buddy_add_free_block(frame, order);
        uint32_t block_size = 1U << order;
        frame += block_size;
        pages -= block_size;
    }
}

static uint32_t buddy_allocate_block(uint8_t order) {
    /* Find or split a free block to satisfy an allocation of the requested
     * order.
     */
    for (uint8_t current_order = order; current_order <= BUDDY_MAX_ORDER; current_order++) {
        if (buddy_free_list[current_order] == UINT32_MAX) {
            continue;
        }

        uint32_t frame = buddy_pop_free_block(current_order);

        while (current_order > order) {
            current_order--;
            uint32_t buddy_frame = frame + (1U << current_order);
            if (((1U << order) * BUDDY_PAGE_SIZE) < UNSPLITTABLE_SIZE)
            {
                return BUDDY_INVALID_ADDRESS;
            }
            
            buddy_add_free_block(buddy_frame, current_order);
        }

        buddy_mark_block_allocated(frame, order);
        return frame;
    }

    return UINT32_MAX;
}

void buddy_init(struct limine_memmap_response *response) {
    /* Initialize buddy allocator state and add usable physical memory ranges. */
    buddy_reset();

    if (response == NULL || response->entry_count == 0 || response->entries == NULL) {
        return;
    }

    for (uint64_t i = 0; i < response->entry_count; i++) {
        struct limine_memmap_entry *entry = response->entries[i];

        if (entry == NULL || entry->type != LIMINE_MEMMAP_USABLE) {
            continue;
        }

        uint64_t start = buddy_align_up(entry->base, BUDDY_PAGE_SIZE);
        uint64_t end = buddy_align_down(entry->base + entry->length, BUDDY_PAGE_SIZE);

        if (end <= start) {
            continue;
        }

        uint64_t first_frame = start / BUDDY_PAGE_SIZE;
        uint64_t last_frame = end / BUDDY_PAGE_SIZE;

        if (first_frame >= BUDDY_MAX_PAGES) {
            continue;
        }

        if (last_frame > BUDDY_MAX_PAGES) {
            last_frame = BUDDY_MAX_PAGES;
        }

        buddy_add_range((uint32_t)first_frame, (uint32_t)(last_frame - first_frame));
    }
}

uintptr_t buddy_alloc_pages(size_t pages) {
    /* Allocate the smallest buddy block that can hold `pages` pages. */
    if (pages == 0 || pages > (1ULL << BUDDY_MAX_ORDER)) {
        return BUDDY_INVALID_ADDRESS;
    }

    uint8_t order = buddy_order_from_pages(pages);
    uint32_t frame = buddy_allocate_block(order);

    if (frame == UINT32_MAX) {
        return BUDDY_INVALID_ADDRESS;
    }

    return (uintptr_t)frame * BUDDY_PAGE_SIZE;
}

void buddy_free_pages(uintptr_t phys_addr, size_t pages) {
    /* Free a previously allocated buddy block and coalesce with free buddies. */
    if (phys_addr == 0 || phys_addr == BUDDY_INVALID_ADDRESS || pages == 0) {
        return;
    }

    if ((phys_addr % BUDDY_PAGE_SIZE) != 0) {
        return;
    }

    uint32_t frame = (uint32_t)(phys_addr / BUDDY_PAGE_SIZE);

    if (frame >= BUDDY_MAX_PAGES) {
        return;
    }

    if (buddy_page_state[frame] != BUDDY_PAGE_STATE_ALLOCATED) {
        return;
    }

    uint8_t order = buddy_order_from_pages(pages);

    while (order < BUDDY_MAX_ORDER) {
        uint32_t buddy_frame = frame ^ (1U << order);

        if (buddy_frame >= BUDDY_MAX_PAGES) {
            break;
        }

        if (buddy_page_state[buddy_frame] != order) {
            break;
        }

        if (!buddy_remove_free_block(buddy_frame, order)) {
            break;
        }

        frame = frame < buddy_frame ? frame : buddy_frame;
        order++;
    }

    buddy_add_free_block(frame, order);
}

uintptr_t buddy_alloc_page(void) {
    return buddy_alloc_pages(1);
}

void buddy_free_page(uintptr_t phys_addr) {
    buddy_free_pages(phys_addr, 1);
}
