/*
 * vmpinix Architecture Subsystem - 64.h
 * x86_64 Long Mode and CPU Control Header
 */

#ifndef ARCH_64BIT_H
#define ARCH_64BIT_H

#define X86_MSR_EFER 0xC0000080
#define X86_EFER_LME (1 << 8)
#define X86_EFER_LMA (1 << 10)
#define X86_EFER_NX  (1 << 11)

typedef struct {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed)) gdt_entry_64_t;

typedef struct {
    unsigned short limit;
    unsigned long long base;
} __attribute__((packed)) gdt_ptr_64_t;

typedef struct {
    unsigned short offset_low;
    unsigned short selector;
    unsigned char ist;
    unsigned char type_attr;
    unsigned short offset_middle;
    unsigned int offset_high;
    unsigned int zero;
} __attribute__((packed)) idt_entry_64_t;

typedef struct {
    unsigned short limit;
    unsigned long long base;
} __attribute__((packed)) idt_ptr_64_t;

void arch_64_init(void);
void arch_64_outb(unsigned short port, unsigned char val);
unsigned char arch_64_inb(unsigned short port);
unsigned long long arch_64_read_msr(unsigned long msr);
void arch_64_write_msr(unsigned long msr, unsigned long long val);
void arch_64_set_gdt(void);
void arch_64_set_idt(void);
unsigned long long arch_64_read_cr3(void);
void arch_64_write_cr3(unsigned long long val);

#endif