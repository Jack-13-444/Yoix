#include <system/interrupt/irq/pit.h>

volatile uint64_t tick; 
void init_pit(uint32_t frequency_hz) {
    // 1. Calculate divisor
    uint32_t divisor = PIT_BASE_FREQ / frequency_hz;
    if (divisor > 0xFFFF) divisor = 0xFFFF; // Cap to 16-bit max
    if (divisor == 0) divisor = 1;

    // 2. Write Command Byte: 
    // Bit 7-6: 00   -> Select Channel 0
    // Bit 5-4: 11   -> Access Mode: Lobyte/Hibyte
    // Bit 3-1: 010  -> Mode 2 (Rate Generator)
    // Bit 0  : 0    -> Binary mode 16-bit
    // Command Value = 0x36
    outb(PIT_COMMAND_REG, 0x36);

    // 3. Send Divisor (Low byte then High byte)
    outb(PIT_CHANNEL0_DATA, (uint8_t)(divisor & 0xFF));        // Low byte
    outb(PIT_CHANNEL0_DATA, (uint8_t)((divisor >> 8) & 0xFF)); // High byte
}
