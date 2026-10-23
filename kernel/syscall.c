/*
 * vmpinix Core Kernel - syscall.c
 * System Call Dispatcher and User-to-Kernel Space Bridge Implementation
 */

#include "../include/stdint.h"

// Çekirdek içi servis referansları
extern int systemctl_execute(const char *command, const char *target);
extern int do_mount(const char *source, const char *target, const char *fstype, unsigned long flags, unsigned int dev_id);
extern int do_unmount(const char *target);
extern void syslog_log(int level, const char *message);

#define SYSCALL_SYSCTL  1
#define SYSCALL_MOUNT   2
#define SYSCALL_UNMOUNT 3
#define SYSCALL_SYSLOG  4

// IDT Girişi Yapısı (Interrupt Gate)
typedef struct {
    unsigned short offset_low;
    unsigned short selector;
    unsigned char zero;
    unsigned char flags;
    unsigned short offset_high;
} __attribute__((packed)) idt_gate_desc_t;

typedef struct {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed)) idt_ptr_desc_t;

// Sistem çağrısı yönlendirme (Dispatcher) tablosu
long syscall_dispatcher(unsigned long syscall_number, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4) {
    switch (syscall_number) {
        case SYSCALL_SYSCTL:
            return (long)systemctl_execute((const char*)arg1, (const char*)arg2);

        case SYSCALL_MOUNT:
            return (long)do_mount((const char*)arg1, (const char*)arg2, (const char*)arg3, (unsigned long)arg4, 0);

        case SYSCALL_UNMOUNT:
            return (long)do_unmount((const char*)arg1);

        case SYSCALL_SYSLOG:
            syslog_log((int)arg1, (const char*)arg2);
            return 0;

        default:
            return -1; // Bilinmeyen sistem çağrısı (ENOSYS)
    }
}

// Donanım seviyesinde 0x80 kesme kapısını IDT'ye işleyen gerçek başlatıcı
void syscall_init(void) {
    idt_ptr_desc_t idt_pointer;
    __asm__ volatile ("sidt %0" : "=m"(idt_pointer));

    idt_gate_desc_t *target_gate = (idt_gate_desc_t*)(idt_pointer.base + (0x80 * sizeof(idt_gate_desc_t)));

    unsigned long dispatcher_address = (unsigned long)syscall_dispatcher;

    target_gate->offset_low = (unsigned short)(dispatcher_address & 0xFFFF);
    target_gate->selector = 0x08; // Çekirdek kod segmenti
    target_gate->zero = 0;
    target_gate->flags = 0xEE;    // Present, DPL 3, 32-bit Interrupt Gate
    target_gate->offset_high = (unsigned short)((dispatcher_address >> 16) & 0xFFFF);
}