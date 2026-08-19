#include <stdbool.h>
#include <stdint.h>

#ifndef _APIC_H
#define _APIC_H

bool check_apic();
void cpu_set_apic_base(uintptr_t apic);
uintptr_t cpu_get_apic_base();
void write_lapic_register(uintptr_t lapic_base, uint32_t offset, uint32_t val);

void enable_apic();
void disable_apic();
void APIC_EOI();

void write_lapic_register(uintptr_t lapic_base, uint32_t offset, uint32_t val);

uint32_t read_lapic_register(uintptr_t lapic_base, uint32_t offset);

#endif