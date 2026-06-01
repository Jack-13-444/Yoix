#include <stdint.h>
#include <memory/tss.h>

#include <stdio.h>
#include <stdint.h>
#ifndef _GDT_H
#define _GDT_H
/* Setting Accessed bit to 1 to prevent potential page faults on read-only GDT memory. */


#define FLAG_DATA_64        0xC
#define FLAG_CODE_64        0xA

#define GDT_COUNT           5

#define KERNEL_CODE         0x9A
#define KERNEL_DATA         0x92
#define USER_CODE           0xF2
#define USER_DATA           0xFA

#define TSS_ACCESS_BYTE     0x89

#define GDT0_NULL_SEGMENT    0x00
#define GDT0_CODE_SEGMENT    0x08
#define GDT0_DATA_SEGMENT    0x10
#define GDT3_DATA_SEGMENT    0x18
#define GDT3_CODE_SEGMENT    0x20
#define GDT0_TSS_SEGMENT     0x28

typedef struct
{
    uint16_t limit;
    uint64_t base;
    
}__attribute__((packed)) GDTR_t;

typedef struct
{
    uint16_t limitL;
    uint16_t baseL;
    uint8_t baseM;    
    uint8_t access;
    uint8_t flags;
    uint8_t baseH;
}__attribute__((packed)) GDT_t;

typedef struct
{
    GDT_t Tmain;
    uint32_t baseTss;
    uint32_t Reserved ;
}__attribute__((packed)) GDT_TSS_t;

typedef struct 
{
    GDTR_t ptr;
    __attribute__((aligned(16))) GDT_t entry[GDT_COUNT];
    GDT_TSS_t entryTss;    
    __attribute__((aligned(16))) TSS_t tss;

}__attribute__((packed)) YOIX_GDT;

void encodeGDTEntry(GDT_t* source, uint64_t base, uint64_t limit, uint8_t access_Byte, uint8_t flags);

void encodeTSSEntry(GDT_TSS_t* source, uint64_t base, uint64_t limit, uint8_t access_Byte, uint8_t flags);
void reloadSegments();
void flush_tss();
void initGdt();
#endif