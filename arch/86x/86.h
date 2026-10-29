/*
 * vmpinix Architecture Subsystem - 86.h
 * Common x86 Architecture Utilities and CPUID Header
 */

#ifndef ARCH_86_H
#define ARCH_86_H

typedef struct {
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
} x86_cpuid_t;

typedef struct {
    unsigned short es;
    unsigned short ds;
    unsigned short fs;
    unsigned short gs;
    unsigned int edi;
    unsigned int esi;
    unsigned int ebp;
    unsigned int esp;
    unsigned int ebx;
    unsigned int edx;
    unsigned int ecx;
    unsigned int eax;
    unsigned int int_no;
    unsigned int err_code;
    unsigned int eip;
    unsigned short cs;
    unsigned int eflags;
} __attribute__((packed)) x86_registers_t;

void arch_86_cpuid(unsigned int code, x86_cpuid_t *out);
unsigned long long arch_86_rdtsc(void);
void arch_86_io_wait(void);
void arch_86_enable_interrupts(void);
void arch_86_disable_interrupts(void);
void arch_86_halt(void);

#endif