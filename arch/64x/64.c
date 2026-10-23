/*
 * vmpinix Architecture Subsystem - 64.c
 * x86_64 Long Mode Management Implementation
 */

#include "64.h"

static gdt_entry_64_t gdt_entries_64[5];
static gdt_ptr_64_t gdt_ptr_64;
static idt_entry_64_t idt_entries_64[256];
static idt_ptr_64_t idt_ptr_64;

void arch_64_outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

unsigned char arch_64_inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

unsigned long long arch_64_read_msr(unsigned long msr) {
    unsigned int low, high;
    __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((unsigned long long)high << 32) | low;
}

void arch_64_write_msr(unsigned long msr, unsigned long long val) {
    unsigned int low = (unsigned int)(val & 0xFFFFFFFF);
    unsigned int high = (unsigned int)(val >> 32);
    __asm__ volatile ("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

unsigned long long arch_64_read_cr0(void) {
    unsigned long long val;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(val));
    return val;
}

void arch_64_write_cr0(unsigned long long val) {
    __asm__ volatile ("mov %0, %%cr0" : : "r"(val));
}

unsigned long long arch_64_read_cr3(void) {
    unsigned long long val;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(val));
    return val;
}

void arch_64_write_cr3(unsigned long long val) {
    __asm__ volatile ("mov %0, %%cr3" : : "r"(val));
}

unsigned long long arch_64_read_cr4(void) {
    unsigned long long val;
    __asm__ volatile ("mov %%cr4, %0" : "=r"(val));
    return val;
}

void arch_64_write_cr4(unsigned long long val) {
    __asm__ volatile ("mov %0, %%cr4" : : "r"(val));
}

static void set_gdt_gate_64(int num, unsigned int base, unsigned int limit,
                           unsigned char access, unsigned char gran) {
    gdt_entries_64[num].base_low = (base & 0xFFFF);
    gdt_entries_64[num].base_middle = (base >> 16) & 0xFF;
    gdt_entries_64[num].base_high = (base >> 24) & 0xFF;
    gdt_entries_64[num].limit_low = (limit & 0xFFFF);
    gdt_entries_64[num].granularity = ((limit >> 16) & 0x0F);
    gdt_entries_64[num].granularity |= (gran & 0xF0);
    gdt_entries_64[num].access = access;
}

void arch_64_set_gdt(void) {
    gdt_ptr_64.limit = (sizeof(gdt_entry_64_t) * 5) - 1;
    gdt_ptr_64.base = (unsigned long long)&gdt_entries_64;

    set_gdt_gate_64(0, 0, 0, 0, 0);                // Null Segment
    set_gdt_gate_64(1, 0, 0xFFFFF, 0x9A, 0xAF);     // Kernel Code
    set_gdt_gate_64(2, 0, 0xFFFFF, 0x92, 0xCF);     // Kernel Data
    set_gdt_gate_64(3, 0, 0xFFFFF, 0xFA, 0xAF);     // User Code
    set_gdt_gate_64(4, 0, 0xFFFFF, 0xF2, 0xCF);     // User Data

    __asm__ volatile ("lgdt (%0)" : : "r"(&gdt_ptr_64));
}

void arch_64_set_idt(void) {
    idt_ptr_64.limit = sizeof(idt_entry_64_t) * 256 - 1;
    idt_ptr_64.base = (unsigned long long)&idt_entries_64;

    for (int i = 0; i < 256; i++) {
        idt_entries_64[i].offset_low = 0;
        idt_entries_64[i].selector = 0x08;
        idt_entries_64[i].ist = 0;
        idt_entries_64[i].type_attr = 0x8E;
        idt_entries_64[i].offset_middle = 0;
        idt_entries_64[i].offset_high = 0;
        idt_entries_64[i].zero = 0;
    }

    __asm__ volatile ("lidt (%0)" : : "r"(&idt_ptr_64));
}

void arch_64_init(void) {
    unsigned long long cr0 = arch_64_read_cr0();
    cr0 &= ~(1ULL << 2);
    cr0 |= (1ULL << 1);
    arch_64_write_cr0(cr0);

    unsigned long long cr4 = arch_64_read_cr4();
    cr4 |= (1ULL << 9);   // OSFXSR
    cr4 |= (1ULL << 10);  // OSXMMEXCPT
    cr4 |= (1ULL << 18);  // OSXSAVE
    arch_64_write_cr4(cr4);

    arch_64_set_gdt();
    arch_64_set_idt();
}