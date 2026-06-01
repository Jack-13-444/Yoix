#include <stdint.h>
#ifndef _TSS_H
#define _TSS_H

typedef struct tss
{
    uint32_t Reserved;
    uint64_t RSP0;
    uint64_t RSP1;
    uint64_t RSP2;
    uint64_t Reserved1;
    uint64_t ist[7];
    uint64_t Reserved2;
    uint32_t Reserved3;
    uint32_t IOPB;
}__attribute__((packed)) TSS_t;


#endif