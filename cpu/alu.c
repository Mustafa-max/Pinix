/*
 * vmpinix CPU Subsystem - alu.c
 * Arithmetic Logic Unit (ALU) Extension Detection, Floating Point, and SIMD Verification
 * x86_64 compatible
 */

#include <stdint.h>

typedef struct {
    int has_fpu;
    int has_sse;
    int has_sse2;
    int has_avx;
    int has_avx2;
    int has_bmi1;
    int has_bmi2;
} alu_features_t;

static alu_features_t cpu_alu_caps;

static int str_eq(const char *a, const char *b) {
    while (*a != '\0' && *b != '\0') {
        if (*a != *b) return 0;
        a++;
        b++;
    }
    return *a == *b;
}

int cpu_alu_init(void) {
    uint32_t eax, ebx, ecx, edx;

    // Leaf 1: processor features
    __asm__ volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1), "c"(0)
    );

    cpu_alu_caps.has_fpu  = (edx & (1u << 0))  ? 1 : 0;
    cpu_alu_caps.has_sse  = (edx & (1u << 25)) ? 1 : 0;
    cpu_alu_caps.has_sse2 = (edx & (1u << 26)) ? 1 : 0;
    cpu_alu_caps.has_avx  = (ecx & (1u << 28)) ? 1 : 0;

    // Leaf 7, subleaf 0: advanced features
    __asm__ volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(7), "c"(0)
    );

    cpu_alu_caps.has_avx2 = (ebx & (1u << 5))  ? 1 : 0;
    cpu_alu_caps.has_bmi1 = (ebx & (1u << 3))  ? 1 : 0;
    cpu_alu_caps.has_bmi2 = (ebx & (1u << 8))  ? 1 : 0;

    // Enable x87 FPU + SSE/AVX support
    uint64_t cr0, cr4;

    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1ULL << 2); // EM bit clear
    cr0 |=  (1ULL << 1); // MP bit set
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));

    __asm__ volatile ("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1ULL << 9);  // OSFXSR
    cr4 |= (1ULL << 10); // OSXMMEXCPT
    if (cpu_alu_caps.has_avx) {
        cr4 |= (1ULL << 18); // OSXSAVE
    }
    __asm__ volatile ("mov %0, %%cr4" : : "r"(cr4));

    return 0;
}

int cpu_alu_check_extension(const char *ext_name) {
    if (!ext_name) return 0;

    // x86_64 default minimum: SSE2 is present on nearly all 64-bit CPUs
    if (str_eq(ext_name, "fpu")) return cpu_alu_caps.has_fpu;
    if (str_eq(ext_name, "sse")) return cpu_alu_caps.has_sse;
    if (str_eq(ext_name, "sse2")) return cpu_alu_caps.has_sse2;
    if (str_eq(ext_name, "avx")) return cpu_alu_caps.has_avx;
    if (str_eq(ext_name, "avx2")) return cpu_alu_caps.has_avx2;
    if (str_eq(ext_name, "bmi1")) return cpu_alu_caps.has_bmi1;
    if (str_eq(ext_name, "bmi2")) return cpu_alu_caps.has_bmi2;

    return 0;
}