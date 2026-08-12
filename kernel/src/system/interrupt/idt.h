#include <stdbool.h>
#include <stdint.h>

#ifndef _IDT_H
#define _IDT_H

typedef struct {
	uint16_t    isr_low;      // The lower 16 bits of the ISR's address
	uint16_t    kernel_cs;    // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t	    ist;          // The IST in the TSS that the CPU will load into RSP; set to zero for now
	uint8_t     attributes;   // Type and attributes; see the IDT page
	uint16_t    isr_mid;      // The higher 16 bits of the lower 32 bits of the ISR's address
	uint32_t    isr_high;     // The higher 32 bits of the ISR's address
	uint32_t    reserved;     // Set to zero
} __attribute__((packed)) idt_entry_t;

typedef struct {
	uint16_t	limit;
	uint64_t	base;
} __attribute__((packed)) idtr_t;

typedef struct 
{
    // in the reverse order they are pushed (For 64-bit):
	uint64_t ds;
	uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
	uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
	uint64_t vectors;
	uint64_t errorcode ,rip, cs, rflags, rsp, ss; 


} __attribute__((packed)) Registers;
void interrupt_handler(Registers *Reg); 

void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags);
void idt_init(void);

#endif