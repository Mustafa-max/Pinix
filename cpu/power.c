/*
 * vmpinix CPU Subsystem - power.c
 * CPU Power Management, C-States, P-States, and Thermal Monitoring Implementation
 */

#include "../kernel/cpu.h"

#define MSR_IA32_PERF_STATUS 0x198
#define MSR_IA32_THERM_STATUS 0x19C
#define MSR_PKG_POWER_LIMIT 0x610

static unsigned long long cpu_read_msr(unsigned long msr) {
    unsigned int low, high;
    __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((unsigned long long)high << 32) | low;
}

static void cpu_write_msr(unsigned long msr, unsigned long long val) {
    unsigned int low = (unsigned int)(val & 0xFFFFFFFF);
    unsigned int high = (unsigned int)(val >> 32);
    __asm__ volatile ("wrmsr" : : "c"(msr), "a"(low), "d"(high));
}

int cpu_power_init(void) {
    // Güç yönetimi denetleyicisi hazırlığı
    return 0;
}

void cpu_enter_cstate(int cstate) {
    if (cstate == 1) {
        __asm__ volatile ("sti; hlt");
    } else if (cstate >= 2) {
        // MWAIT tabanlı derin uyku durumu simülasyonu
        unsigned int eax = 0x20; // C2 State
        unsigned int ecx = 0;
        __asm__ volatile ("mwait" : : "a"(eax), "c"(ecx));
    }
}

unsigned int cpu_get_current_frequency_mhz(void) {
    unsigned long long status = cpu_read_msr(MSR_IA32_PERF_STATUS);
    // Basit çarpan tabanlı frekans hesabı (Örn: 100MHz * Basamak)
    unsigned int multiplier = (unsigned int)((status >> 8) & 0xFF);
    if (multiplier == 0) multiplier = 20; // Varsayılan emniyet değeri
    return multiplier * 100;
}

int cpu_get_temperature_celsius(void) {
    unsigned long long therm = cpu_read_msr(MSR_IA32_THERM_STATUS);
    if (!(therm & (1 << 31))) {
        return -1; // Sensör geçerli değil
    }
    // Dijital okuma (TJMax - delta)
    unsigned int delta = (unsigned int)((therm >> 16) & 0x7F);
    int tj_max = 100; // Standart çekirdek max sıcaklığı
    return tj_max - (int)delta;
}