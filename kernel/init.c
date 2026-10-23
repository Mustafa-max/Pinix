/*
 * vmpinix Core Kernel - init.c
 * Subsystem Initialization and Bootstrapping Integration
 */

#include "../mm/memory.h"

// 64-bit mimari prototipleri
extern void arch_64_init(void);
extern void arch_64_set_gdt(void);
extern void arch_64_set_idt(void);

// CPU prototipleri
extern void intel_cpu_init(void);
extern void amd_cpu_init(void);
extern int cpu_subsystem_init(void);

// Cihaz ve Bus alt sistem prototipleri
extern int devices_subsystem_init(void);
extern int bus_subsystem_init(void);
extern int class_subsystem_init(void);
extern int devfs_init(void);

// Grafik prototipleri
extern int vesa_init(void);

// Dosya sistemi mount prototipleri
extern int ext4_mount(unsigned int dev_id);
extern int ntfs3_mount(unsigned int dev_id);

// Zamanlayıcı ve Syscall prototipleri
extern void cpu_timer_init(unsigned int frequency);
extern void syscall_init(void);

void arch_init(void) {
    arch_64_init();
}

void cpu_init(void) {
    // Intel/AMD ayrı ayrı initler varsa burası çağrılır.
    // Burada tek bir CPU path'i kullanılırsa aşağıdakiler uygun.
    intel_cpu_init();
    amd_cpu_init();
    cpu_subsystem_init();
}

void devices_init(void) {
    devices_subsystem_init();
    bus_subsystem_init();
    class_subsystem_init();
    devfs_init();
}

void drivers_init(void) {
    // VGA metin modu devre dışı bırakıldı.
    // KMS / VESA / framebuffer sadece burada başlatılır.
    vesa_init();
}

void timer_init(void) {
    cpu_timer_init(100); // 100 Hz
}

void fs_subsystem_init(void) {
    ext4_mount(0);
    ntfs3_mount(0);
}