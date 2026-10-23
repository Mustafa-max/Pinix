/*
 * vmpinix CPU Subsystem - smp.c
 * Symmetric Multiprocessing (SMP) Initialization, APIC Boot, and Core Synchronization
 */

#include "../kernel/cpu.h"

#define APIC_ICR_LOW   (volatile unsigned int*)0xFEE00300
#define APIC_ICR_HIGH  (volatile unsigned int*)0xFEE00310
#define APIC_SVR       (volatile unsigned int*)0xFEE000F0

typedef struct {
    unsigned int apic_id;
    unsigned int stack_top;
    unsigned int active;
    unsigned int initialized;
} smp_core_context_t;

static smp_core_context_t smp_context_table[256];
static unsigned int smp_active_cores = 1;

int smp_subsystem_init(void) {
    for (int i = 0; i < 256; i++) {
        smp_context_table[i].apic_id = i;
        smp_context_table[i].stack_top = 0;
        smp_context_table[i].active = 0;
        smp_context_table[i].initialized = 0;
    }

    // BSP (Bootstrap Processor) kaydı
    smp_context_table[0].apic_id = 0;
    smp_context_table[0].active = 1;
    smp_context_table[0].initialized = 1;
    smp_active_cores = 1;

    // Spurious Interrupt Vector Register aktif etme
    *APIC_SVR = (*APIC_SVR | 0x100); // APIC Yazılım Etkinleştirme (Bit 8)

    return 0;
}

int smp_boot_ap(unsigned int apic_id, unsigned int vector_page) {
    if (apic_id == 0 || apic_id >= 256) {
        return -1;
    }

    // 1. INIT IPI Gönder (Assert, Trigger Level)
    *APIC_ICR_HIGH = (apic_id << 24);
    *APIC_ICR_LOW = 0x00004500; // Delivery Mode: INIT, Level: Assert, Trigger: Edge

    for (volatile int i = 0; i < 10000; i++) {
        __asm__ volatile ("pause");
    }

    // 2. Startup IPI (SIPI) Gönder
    *APIC_ICR_HIGH = (apic_id << 24);
    *APIC_ICR_LOW = 0x00004600 | (vector_page & 0xFF); // Delivery Mode: Startup

    // İkinci SIPI gönderimi (güvenilirlik için standart prosedür)
    *APIC_ICR_HIGH = (apic_id << 24);
    *APIC_ICR_LOW = 0x00004600 | (vector_page & 0xFF);

    smp_context_table[apic_id].active = 1;
    smp_context_table[apic_id].initialized = 1;
    smp_active_cores++;

    return 0;
}

unsigned int smp_get_active_core_count(void) {
    return smp_active_cores;
}

int smp_is_core_active(unsigned int apic_id) {
    if (apic_id >= 256) return 0;
    return smp_context_table[apic_id].active;
}