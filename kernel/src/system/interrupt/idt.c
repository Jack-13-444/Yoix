#include "system/interrupt/idt.h"
#include <stdbool.h>
#include <stdint.h>
#include <memory/buddy.h>
#include <memory/heap.h>
#include <memory/vmm.h>
#include <system/interrupt/APIC/apic.h>

__attribute__((aligned(0x10))) 
static idt_entry_t idt[256]; // Create an array of IDT entries; aligned for performance

static idtr_t idtr;
static bool vectors[256];

extern void* isr_stub_table[];
extern volatile uint64_t tick;

static const char* const g_Exceptions[] = {
    "Divide by zero error",
    "Debug",
    "Non-maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception ",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "Reserved"
};


void interrupt_handler(Registers *Reg)
{
    if (Reg->vectors > 31 && Reg->vectors < 48)
    {
        irq_handler(Reg);
        APIC_EOI();        
    }
    if (Reg->vectors < 32)
    { // not have more of handling for expections
        __asm__ volatile ("cli; hlt"); // Completely hangs the computer
    }

    return;
}
void irq_handler(Registers *Reg)
{    
    switch (Reg->vectors)
    {
    case 0x20:
        tick++;
        break;
    
    default:
        break;
    }
    return ;
}


void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    idt_entry_t* descriptor = &idt[vector];

    descriptor->isr_low        = (uint64_t)isr & 0xFFFF;
    descriptor->kernel_cs      = 0x08; // Kernel code segment selector
    descriptor->ist            = 0;
    descriptor->attributes     = flags;
    descriptor->isr_mid        = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor->isr_high       = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved       = 0;
}


void idt_init() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.limit = (uint16_t)sizeof(idt_entry_t) * 256 - 1;

    for (uint8_t vector = 0; vector < 49; vector++) {
        idt_set_descriptor(vector, isr_stub_table[vector], 0x8E);
        vectors[vector] = true;
    }
    idt_set_descriptor(255, isr_stub_table[255], 0x8E);
    vectors[255] = true;
    
    __asm__ volatile ("lidt %0" : : "m"(idtr)); // load the new IDT
    __asm__ volatile ("sti"); // set the interrupt flag
}