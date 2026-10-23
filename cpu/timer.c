/*
 * vmpinix CPU Subsystem - timer.c
 * High-Precision Local APIC and PIT Timer Subsystem Implementation
 */

#include "outb.h"

#define PIT_CHANNEL0_PORT 0x40
#define PIT_COMMAND_PORT  0x43
#define PIT_BASE_FREQ     1193182

static volatile unsigned long long cpu_system_ticks = 0;
static unsigned int current_timer_frequency = 0;

void cpu_timer_init(unsigned int frequency) {
    if (frequency == 0) return;
    current_timer_frequency = frequency;
    unsigned int divisor = PIT_BASE_FREQ / frequency;

    outb(PIT_COMMAND_PORT, 0x36);
    outb(PIT_CHANNEL0_PORT, (unsigned char)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (unsigned char)((divisor >> 8) & 0xFF));

    cpu_system_ticks = 0;
}

void cpu_timer_interrupt_handler(void) {
    cpu_system_ticks++;
}

unsigned long long cpu_timer_get_ticks(void) {
    return cpu_system_ticks;
}

void cpu_timer_delay_ms(unsigned long milliseconds) {
    if (current_timer_frequency == 0) return;
    unsigned long long target_ticks = cpu_system_ticks + ((milliseconds * current_timer_frequency) / 1000);
    while (cpu_system_ticks < target_ticks) {
        __asm__ volatile ("pause");
    }
}

unsigned int cpu_timer_get_frequency(void) {
    return current_timer_frequency;
}