#include <stdint.h>

#define DRIVER_OK 0
#define DRIVER_ERR -1

typedef struct {
    uint64_t framebuffer_base;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint32_t framebuffer_bpp;
    uint32_t boot_mode;
    char cpu_vendor[16];
    char system_vendor[64];
} boot_info_t;

typedef struct {
    const char *name;
    int (*probe)(void);
    int (*init)(void);
} driver_t;

static driver_t g_drivers[16];
static uint32_t g_driver_count = 0;

static void register_driver(const char *name, int (*probe)(void), int (*init)(void)) {
    if (g_driver_count >= 16) {
        return;
    }

    g_drivers[g_driver_count].name = name;
    g_drivers[g_driver_count].probe = probe;
    g_drivers[g_driver_count].init = init;
    g_driver_count++;
}

static int gpu_probe(void) {
    return DRIVER_OK;
}

static int gpu_init(void) {
    return DRIVER_OK;
}

static int ps2_probe(void) {
    return DRIVER_OK;
}

static int ps2_init(void) {
    return DRIVER_OK;
}

static int ide_probe(void) {
    return DRIVER_OK;
}

static int ide_init(void) {
    return DRIVER_OK;
}

static void driver_init_all(void) {
    for (uint32_t i = 0; i < g_driver_count; i++) {
        if (g_drivers[i].probe && g_drivers[i].probe() == DRIVER_OK) {
            if (g_drivers[i].init) {
                g_drivers[i].init();
            }
        }
    }
}

static void cpuid_vendor(char out[13]) {
    uint32_t eax, ebx, ecx, edx;

    __asm__ volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0)
    );

    char *p = out;

    *p++ = (char)(ebx & 0xFF);
    *p++ = (char)((ebx >> 8) & 0xFF);
    *p++ = (char)((ebx >> 16) & 0xFF);
    *p++ = (char)((ebx >> 24) & 0xFF);

    *p++ = (char)(edx & 0xFF);
    *p++ = (char)((edx >> 8) & 0xFF);
    *p++ = (char)((edx >> 16) & 0xFF);
    *p++ = (char)((edx >> 24) & 0xFF);

    *p++ = (char)(ecx & 0xFF);
    *p++ = (char)((ecx >> 8) & 0xFF);
    *p++ = (char)((ecx >> 16) & 0xFF);
    *p++ = (char)((ecx >> 24) & 0xFF);

    *p = '\0';
}

static void detect_cpu_vendor(boot_info_t *info) {
    char vendor[13] = {0};
    cpuid_vendor(vendor);

    if (vendor[0] == 'G' && vendor[1] == 'e' && vendor[2] == 'n' && vendor[3] == 'u') {
        __builtin_memcpy(info->cpu_vendor, "Intel", 6);
    } else if (vendor[0] == 'A' && vendor[1] == 'u' && vendor[2] == 't' && vendor[3] == 'h') {
        __builtin_memcpy(info->cpu_vendor, "AMD", 4);
    } else {
        __builtin_memcpy(info->cpu_vendor, "Unknown", 8);
    }
}

static void detect_system_vendor(boot_info_t *info) {
    // Gerçek HP/Dell/Intel/AMD tespiti burada SMBIOS/ACPI/UEFI sistem table ile yapılır.
    // Şimdilik generic olarak bırakıyoruz.
    __builtin_memcpy(info->system_vendor, "HP/Dell Generic", 16);
}

static void fill_screen(uint32_t *fb, uint32_t width, uint32_t height, uint32_t pitch, uint32_t color) {
    uint32_t pitch_px = pitch / 4;

    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            fb[(y * pitch_px) + x] = color;
        }
    }
}

static void fill_rect(uint32_t *fb, uint32_t width, uint32_t height, uint32_t pitch,
                     uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    uint32_t pitch_px = pitch / 4;

    if (x >= width || y >= height) {
        return;
    }

    uint32_t max_x = x + w;
    uint32_t max_y = y + h;

    if (max_x > width) max_x = width;
    if (max_y > height) max_y = height;

    for (uint32_t py = y; py < max_y; ++py) {
        for (uint32_t px = x; px < max_x; ++px) {
            fb[(py * pitch_px) + px] = color;
        }
    }
}

static void init_gpu_and_draw(boot_info_t *info) {
    uint32_t *fb = (uint32_t *)(uintptr_t)info->framebuffer_base;

    if (!fb) {
        return;
    }

    fill_screen(fb, info->framebuffer_width, info->framebuffer_height,
                info->framebuffer_pitch, 0x00FF00);

    uint32_t box_size = 120;
    uint32_t x = info->framebuffer_width - box_size - 20;
    uint32_t y = 20;

    fill_rect(fb, info->framebuffer_width, info->framebuffer_height,
              info->framebuffer_pitch, x, y, box_size, box_size, 0xFFFF00);
}

static void init_arch(void) {
    // CPU/APIC/PIC/VM init burada
}

static void init_memory(void) {
    // memory init burada
}

static void init_devices(void) {
    // PCI/ACPI/bus init burada
}

void kernel_main(void) {
    boot_info_t info = {0};

    // Varsayılan framebuffer bilgisi
    info.framebuffer_base = 0x00000000;
    info.framebuffer_pitch = 1024 * 4;
    info.framebuffer_width = 1024;
    info.framebuffer_height = 768;
    info.framebuffer_bpp = 32;
    info.boot_mode = 0;

    detect_cpu_vendor(&info);
    detect_system_vendor(&info);

    init_arch();
    init_memory();
    init_devices();

    register_driver("gpu", gpu_probe, gpu_init);
    register_driver("ps2", ps2_probe, ps2_init);
    register_driver("ide", ide_probe, ide_init);

    init_gpu_and_draw(&info);

    driver_init_all();

    for (;;) {
        __asm__ volatile ("hlt");
    }
}