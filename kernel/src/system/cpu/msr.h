#include <stdint.h>
#include <stdbool.h>
#include  <system/cpu/cpuid.h>
#ifndef _MSR_H
#define _MSR_H

bool cpuHasMSR();

void cpuGetMSR(uint32_t msr, uint32_t *lo, uint32_t *hi);

void cpuSetMSR(uint32_t msr, uint32_t lo, uint32_t hi);

#endif