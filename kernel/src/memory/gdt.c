// Used for creating GDT segment descriptors in 64-bit integer form.
 
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <boot/limine.h>
#include "gdt.h"
YOIX_GDT gdt; 

void encodeGDTEntry(GDT_t* source, uint64_t base, uint64_t limit, uint8_t access_Byte, uint8_t flags)
{
    source->baseL			=	(base & 0xFFFF);
    source->baseM			=	((base >> 16) & 0xFF);
	source->baseH			=	((base >> 24) & 0xFF);
    source->limitL			=	(limit & 0xFFFF);
	source->access			= 	access_Byte;
	source->flags			=	(((limit >> 16) & 0x0F) | (flags << 4));	
    
}
void encodeTSSEntry(GDT_TSS_t* source, uint64_t base, uint64_t limit, uint8_t access_Byte, uint8_t flags)
{
	encodeGDTEntry(&source->Tmain, base, limit, access_Byte, flags);
	source->baseTss = (base >> 32) & 0xFFFFFFFF;
	source->Reserved = 0;
}

void initGdt()
{
	encodeGDTEntry(&gdt.entry[0],0,0,0,0);
	encodeGDTEntry(&gdt.entry[1],0, 0xFFFFF, KERNEL_CODE, FLAG_CODE_64);	
	encodeGDTEntry(&gdt.entry[2],0, 0xFFFFF, KERNEL_DATA, FLAG_DATA_64);	
	encodeGDTEntry(&gdt.entry[3],0, 0xFFFFF, USER_DATA, FLAG_DATA_64);	
	encodeGDTEntry(&gdt.entry[4],0, 0xFFFFF, USER_CODE, FLAG_CODE_64);
	encodeTSSEntry(&gdt.entryTss, (uint64_t)&gdt.tss,(sizeof(TSS_t) - 1), TSS_ACCESS_BYTE, 0x0);
	
	memset(&gdt.tss, 0 , sizeof(TSS_t));
	gdt.tss.RSP0 = 0;
	gdt.ptr.base 	= (uint64_t)&gdt.entry;
	gdt.ptr.limit	= (sizeof(GDT_t) * GDT_COUNT + sizeof(GDT_TSS_t) - 1);

	asm volatile("lgdt %0" :: "m"(gdt.ptr));
	reloadSegments();
	flush_tss();
}
