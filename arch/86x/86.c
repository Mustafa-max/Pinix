/*
 * vmpinix Architecture Subsystem - 86.c
 * Common x86 Architecture Utilities Implementation
 */

#include "86.h"

void arch_86_cpuid(unsigned int code, x86_cpuid_t *out) {
    if (!out) {
        return;
    }
    __asm__ volatile (
        "cpuid"
        : "=a"(out->eax), "=b"(out->ebx), "=c"(out->ecx), "=d"(out->edx)
        : "a"(code)
    );
}

unsigned long long arch_86_rdtsc(void) {
    unsigned int low, high;
    __asm__ volatile ("rdtsc" : "=a"(low), "=d"(high));
    return ((unsigned long long)high << 32) | low;
}

void arch_86_io_wait(void) {
    __asm__ volatile ("outb %%al, $0x80" : : "a"(0));
}

void arch_86_enable_interrupts(void) {
    __asm__ volatile ("sti");
}

void arch_86_disable_interrupts(void) {
    __asm__ volatile ("cli");
}

void arch_86_halt(void) {
    __asm__ volatile ("hlt");
}