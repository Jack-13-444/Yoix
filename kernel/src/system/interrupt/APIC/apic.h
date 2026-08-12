#include <stdbool.h>
#include <stdint.h>

#ifndef _APIC_H
#define _APIC_H

bool check_apic();
void cpu_set_apic_base(uintptr_t apic);
uintptr_t cpu_get_apic_base();

void enable_apic();
void disable_apic();

#endif