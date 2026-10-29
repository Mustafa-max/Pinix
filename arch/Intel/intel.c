/*
 * vmpinix Intel CPU Subsystem - intel.c
 * Intel Processor Specific Configuration and Initialization
 */

#include "../../kernel/cpu.h"

static unsigned int intel_msr_read(unsigned long msr) {
    unsigned int low, high;
    __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return low;
}

static void intel_msr_write(unsigned long msr, unsigned int low, unsigned int high) {
    __asm__ volatile ("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

void intel_configure_features(void) {
    // Intel işlemcilere özel MSR (Model Specific Register) optimizasyonları
    // Örneğin: Thermal Control veya Turbo Boost bayrakları kontrolü
    unsigned long ia32_misc_enable = 0x1A0;
    unsigned int low = intel_msr_read(ia32_misc_enable);
    
    // Özellikleri etkinleştir (Fast Strings vb.)
    low |= (1 << 0); 
    intel_msr_write(ia32_misc_enable, low, 0);
}

void intel_cpu_init(void) {
    // Intel mimarisine özel donanım kayıtçısı yapılandırmaları
    intel_configure_features();
    
    // C1E (Enhanced Halt State) yapılandırması
    unsigned long ia32_pm_enable = 0x3FC;
    (void)ia32_pm_enable;
}