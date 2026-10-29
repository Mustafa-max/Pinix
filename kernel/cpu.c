/*
 * vmpinix Kernel Subsystem - cpu.c
 * Central CPU Management, Initialization, and Memory Subsystem (mm.c) Bridge
 */

#define NULL ((void*)0)

typedef struct {
    unsigned int apic_id;
    unsigned int package_id;
    unsigned int core_id;
    unsigned int thread_id;
    int active;
} cpu_topology_node_t;

typedef struct {
    unsigned int vendor_ebx;
    unsigned int vendor_edx;
    unsigned int vendor_ecx;
    unsigned int family;
    unsigned int model;
    unsigned int stepping;
    unsigned int apic_id;
    unsigned long long features;
    char vendor_string[13];
} cpu_core_info_t;

// Dış alt sistem referansları (cpu/ ve mm/ arası köprü)
extern int cpu_subsystem_init(void);
extern unsigned int smp_get_active_core_count(void);
extern void cpu_timer_init(unsigned int frequency);
extern int cpu_alu_init(void);
extern int cpu_topology_init(void);

// Bellek yönetimi (mm.c) entegrasyon prototipleri
extern void* mm_allocate_kernel_pages(unsigned int page_count);
extern void mm_free_kernel_pages(void *ptr, unsigned int page_count);
extern unsigned long long mm_get_free_memory_bytes(void);

#define MAX_CPU_CORES 256

typedef struct {
    cpu_core_info_t core_info;
    void *per_cpu_page_table;
    unsigned long long allocated_memory_pool;
    int initialized;
} kernel_cpu_node_t;

static kernel_cpu_node_t kernel_cpus[MAX_CPU_CORES];
static unsigned int registered_cpu_count = 0;
static int central_cpu_subsys_ready = 0;

static void kernel_cpuid_internal(unsigned int code, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx) {
    __asm__ volatile (
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(code), "c"(0)
    );
}

int cpu_subsystem_init(void) {
    if (central_cpu_subsys_ready) {
        return 0;
    }

    registered_cpu_count = 0;
    for (int i = 0; i < MAX_CPU_CORES; i++) {
        kernel_cpus[i].initialized = 0;
        kernel_cpus[i].per_cpu_page_table = NULL;
        kernel_cpus[i].allocated_memory_pool = 0;
    }

    // cpu/ alt klasör modüllerini sırayla ve doğruca tetikle
    cpu_topology_init();
    cpu_alu_init();
    cpu_timer_init(1000); // 1 kHz Sistem Zamanlayıcı

    // BSP (Bootstrap Processor) Bilgilerini Topla
    unsigned int eax, ebx, ecx, edx;
    kernel_cpuid_internal(0, &eax, &ebx, &ecx, &edx);

    kernel_cpu_node_t *bsp = &kernel_cpus[0];
    *(unsigned int*)&bsp->core_info.vendor_string[0] = ebx;
    *(unsigned int*)&bsp->core_info.vendor_string[4] = edx;
    *(unsigned int*)&bsp->core_info.vendor_string[8] = ecx;
    bsp->core_info.vendor_string[12] = '\0';

    kernel_cpuid_internal(1, &eax, &ebx, &ecx, &edx);
    bsp->core_info.stepping = eax & 0xF;
    bsp->core_info.model = (eax >> 4) & 0xF;
    bsp->core_info.family = (eax >> 8) & 0xF;
    bsp->core_info.features = ((unsigned long long)edx) | (((unsigned long long)ecx) << 32);
    bsp->core_info.apic_id = (ebx >> 24) & 0xFF;

    // mm.c (Bellek Alt Sistemi) ile haberleşerek her çekirdek için sayfa tablosu alanı tahsis et
    void *page_table_mem = mm_allocate_kernel_pages(1);
    if (page_table_mem) {
        bsp->per_cpu_page_table = page_table_mem;
        bsp->allocated_memory_pool = 4096; // 1 Sayfa (4KB)
    }

    bsp->initialized = 1;
    registered_cpu_count = 1;
    central_cpu_subsys_ready = 1;

    return 0;
}

cpu_core_info_t* kernel_cpu_get_info(unsigned int core_index) {
    if (core_index >= registered_cpu_count || !kernel_cpus[core_index].initialized) {
        return NULL;
    }
    return &kernel_cpus[core_index].core_info;
}

unsigned int kernel_cpu_get_count(void) {
    return smp_get_active_core_count();
}

int kernel_cpu_synchronize_memory(unsigned int core_index) {
    if (core_index >= registered_cpu_count) {
        return -1;
    }

    // mm.c üzerinden mevcut boş bellek durumunu kontrol et ve senkronize et
    unsigned long long free_mem = mm_get_free_memory_bytes();
    if (free_mem == 0) {
        return -2; // Bellek kritik seviyede
    }

    return 0;
}

void kernel_cpu_exit(void) {
    if (!central_cpu_subsys_ready) {
        return;
    }

    for (unsigned int i = 0; i < registered_cpu_count; i++) {
        if (kernel_cpus[i].per_cpu_page_table) {
            mm_free_kernel_pages(kernel_cpus[i].per_cpu_page_table, 1);
            kernel_cpus[i].per_cpu_page_table = NULL;
        }
        kernel_cpus[i].initialized = 0;
    }

    central_cpu_subsys_ready = 0;
    registered_cpu_count = 0;
}