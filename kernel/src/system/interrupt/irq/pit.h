#include <stdint.h>
#include <system/cpu/io.h>

#define PIT_CHANNEL0_DATA 0x40
#define PIT_COMMAND_REG   0x43
#define PIT_BASE_FREQ     1193182


void init_pit(uint32_t frequency_hz);
