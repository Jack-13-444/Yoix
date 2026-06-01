#include "stddef.h"
#include "memory/pmm.h"
#include "math.h"

struct block blocks[2048];
struct Buddy *buddy;

void memory_map(struct limine_memmap_response *MMap, uint64_t order,struct block *blocks)
{
	// here we will parse the memory map and initialize the buddy system
	// we will use the memory map to find the available memory and initialize the buddy system with it
	size_t block_index = 0;
	for (size_t i = 0; i < MMap->entry_count; i++)
	{
		if (MMap->entries[i]->type == LIMINE_MEMMAP_USABLE)
		{
			blocks[block_index].address				= 	MMap->entries[i]->base;
			blocks[block_index].IsFree				= 	true;
			blocks[block_index].size				=	1ULL << order;
			blocks[block_index].node.prev 			=	NULL;
			blocks[block_index].node.next 			=	NULL;
			block_index++;
		}
		
	}
	
}

void init_buddy(struct limine_memmap_response *MMap)
{
    // init the buddy system
    buddy->Current_Order  		=	11;
	memory_map(MMap, buddy->Current_Order, blocks);

	for (size_t i = 0; i < 12; i++)
	{
		buddy->free_list[i].next = &buddy->free_list[i];
		buddy->free_list[i].prev = &buddy->free_list[i];
		
	}

	buddy->free_list[11].next = &blocks[0].node;
	buddy->free_list[11].prev = &blocks[0].node;
	blocks[0].node.next = &buddy->free_list[11];
	blocks[0].node.prev = &buddy->free_list[11];

}
void split(struct Buddy *Buddy, struct block *Block, uint64_t order)
{
	// split
	if (order >= Buddy->Current_Order)
	{
		return;
	}
	for (size_t i = Buddy->Current_Order; i > order; i--)
	{
		if (Buddy->free_list[i].next != &Buddy->free_list[i])
		{
			struct Free_List *Node				=		Buddy->free_list[i].next;
			struct block *PBlock				=		container_of(Node,struct block, node);

			Block->address						=		(uint64_t)(PBlock->address + (1ULL << (i-1)));
			PBlock->size						=		PBlock->size/2;
			Block->size 						=		PBlock->size;
			PBlock->IsFree						=		true;
			Block->IsFree						=		true;

			Node->next->prev 					= 		Node->prev;
			Node->prev->next 					= 		Node->next;

			Buddy->free_list[i-1].next			=		&Block->node;
			Buddy->free_list[i-1].prev			=		&PBlock->node;

			Block->node.next					=		&Buddy->free_list[i-1];
			Block->node.prev					=		&PBlock->node;

			PBlock->node.next					=		&Block->node;
			PBlock->node.prev					=		&Buddy->free_list[i-1];
		}
	}	
}
void merge(uint64_t *Block, uint64_t order)
{
	uint64_t *buddy_address = (uint64_t*)((uint64_t)Block ^ (1ULL << order));

	for (size_t i = 0; i < 2048; i++)
	{

		struct Free_List *Node					=		&blocks[i].node;
		
		if (blocks[i].address == (uint64_t)buddy_address)
		{
			if (blocks[i].IsFree)
			{
			
			Node->next->prev 					= 		Node->prev;
			Node->prev->next 					= 		Node->next;
			if (blocks[i].address < (uint64_t)Block)
			{
				merge(&blocks[i].address,(order+1));

			}else
			{
				merge(Block,(order+1));
				
			}			
			}		

		}
				
	}		
}

void* kmalloc(size_t size)
{
	if (!buddy) return NULL;
	
	uint64_t order = 63 - __builtin_clzll(size);
	if (size > (1ULL << order)) order++;
	if (buddy->free_list[order].next != &buddy->free_list[order])
	{
		
		struct Free_List *Node					=		buddy->free_list[order].next;
		struct block *PBlock					=		container_of(Node,struct block, node);
		if (PBlock->IsFree)
		{
			PBlock->IsFree						=		false;

			Node->next->prev 					= 		Node->prev;
			Node->prev->next 					= 		Node->next;
			
			return (void *)PBlock->address;
		}	
		
	}
	for (size_t i = 0; i < 2048; i++)
	{
		if (blocks[i].IsFree)
			{
				split(buddy, &blocks[i],order);
				blocks[i].IsFree			=	false;
				return (void*)blocks[i].address;
			}
								
	}

	return NULL;
}
void* kfree(void *ptr)
{
	for (size_t i = 0; i < 2048; i++)
	{
		if (blocks[i].address == (uint64_t)ptr)
		{
			blocks[i].IsFree = true;
		    uint64_t order = __builtin_ctzll(blocks[i].size);

			merge(&blocks[i].address, order);
			return NULL;
		}
								
	}
}
