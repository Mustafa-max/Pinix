/*
 * vmpinix AMD CPU Subsystem - amd.c
 * AMD Processor Specific Configuration and Initialization
 */

#include "../../kernel/cpu.h"

static unsigned int amd_msr_read(unsigned long msr) {
    unsigned int low, high;
    __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return low;
}

static void amd_msr_write(unsigned long msr, unsigned int low, unsigned int high) {
    __asm__ volatile ("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

void amd_configure_features(void) {
    // AMD işlemcilere özel MSR yapılandırmaları (örn. HWCR - Hardware Configuration Register)
    unsigned long amd_hwcr_msr = 0xC0010015;
    unsigned int low = amd_msr_read(amd_hwcr_msr);
    
    // Koy (Fetch) ve önbellek optimizasyon bayrakları
    low |= (1 << 0);
    amd_msr_write(amd_hwcr_msr, low, 0);
}

void amd_cpu_init(void) {
    // AMD mimarisine özel çekirdek ve önbellek yönetimi
    amd_configure_features();
    
    // Zen veya K10+ tabanlı mimariler için ek güvenlik/performans kilitleri
    unsigned long sys_cfg = 0xC0010010;
    (void)sys_cfg;
}