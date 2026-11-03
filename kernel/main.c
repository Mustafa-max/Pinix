#include <stdint.h>

#define PINIX_BOOT_INFO_MAGIC   0x50494E4958424931ULL
#define PINIX_BOOT_INFO_VERSION 1u

#define PINIX_FIRMWARE_UNKNOWN 0u
#define PINIX_FIRMWARE_BIOS    1u
#define PINIX_FIRMWARE_UEFI    2u

/* Bellekte B, G, R, X baytları bulunan 32-bit framebuffer. */
#define PINIX_FB_FORMAT_BGRX 1u

typedef struct {
    uint64_t magic;
    uint32_t version;
    uint32_t struct_size;

    uint32_t firmware_type;
    uint32_t flags;

    uint64_t framebuffer_address;
    uint64_t framebuffer_size;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint32_t framebuffer_pitch_bytes;
    uint32_t framebuffer_bpp;
    uint32_t framebuffer_format;
    uint32_t reserved;
} pinix_boot_info_t;

static __attribute__((noreturn)) void halt_forever(void) {
    for (;;) {
        __asm__ volatile ("cli; hlt" ::: "memory");
    }
}

/* CPUID üretici metnini aynen verir: ör. "GenuineIntel", "AuthenticAMD". */
static void read_cpu_vendor(char out[13]) {
    uint32_t eax, ebx, ecx, edx;

    __asm__ volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0), "c"(0)
    );

    (void)eax;

    out[0] = (char)(ebx);
    out[1] = (char)(ebx >> 8);
    out[2] = (char)(ebx >> 16);
    out[3] = (char)(ebx >> 24);

    out[4] = (char)(edx);
    out[5] = (char)(edx >> 8);
    out[6] = (char)(edx >> 16);
    out[7] = (char)(edx >> 24);

    out[8]  = (char)(ecx);
    out[9]  = (char)(ecx >> 8);
    out[10] = (char)(ecx >> 16);
    out[11] = (char)(ecx >> 24);
    out[12] = '\0';
}

/*
 * Framebuffer adresi, bootloader tarafından kernel'in erişebildiği bir
 * adrese eşlenmiş olmalıdır. Bu fonksiyon yalnızca desteklenen formatı çizer.
 */
static int framebuffer_is_valid(const pinix_boot_info_t *info) {
    if (info->framebuffer_address == 0 ||
        info->framebuffer_size == 0 ||
        info->framebuffer_width == 0 ||
        info->framebuffer_height == 0 ||
        info->framebuffer_bpp != 32 ||
        info->framebuffer_format != PINIX_FB_FORMAT_BGRX) {
        return 0;
    }

    uint64_t row_bytes = (uint64_t)info->framebuffer_width * 4u;
    if (info->framebuffer_pitch_bytes < row_bytes) {
        return 0;
    }

    uint64_t rows_before_last = (uint64_t)info->framebuffer_height - 1u;
    uint64_t max_u64 = ~(uint64_t)0;

    if (rows_before_last != 0 &&
        info->framebuffer_pitch_bytes >
            (max_u64 - row_bytes) / rows_before_last) {
        return 0;
    }

    uint64_t required_size =
        rows_before_last * info->framebuffer_pitch_bytes + row_bytes;

    if (required_size > info->framebuffer_size ||
        info->framebuffer_address > max_u64 - required_size) {
        return 0;
    }

    return 1;
}

static void framebuffer_clear(const pinix_boot_info_t *info, uint32_t color) {
    volatile uint32_t *framebuffer =
        (volatile uint32_t *)(uintptr_t)info->framebuffer_address;
    uint32_t pitch_pixels = info->framebuffer_pitch_bytes / 4u;

    for (uint32_t y = 0; y < info->framebuffer_height; ++y) {
        for (uint32_t x = 0; x < info->framebuffer_width; ++x) {
            framebuffer[(uint64_t)y * pitch_pixels + x] = color;
        }
    }
}

static void framebuffer_fill_rect(
    const pinix_boot_info_t *info,
    uint32_t x, uint32_t y,
    uint32_t width, uint32_t height,
    uint32_t color
) {
    if (!framebuffer_is_valid(info) ||
        x >= info->framebuffer_width ||
        y >= info->framebuffer_height) {
        return;
    }

    uint64_t right = (uint64_t)x + width;
    uint64_t bottom = (uint64_t)y + height;

    if (right > info->framebuffer_width) {
        right = info->framebuffer_width;
    }
    if (bottom > info->framebuffer_height) {
        bottom = info->framebuffer_height;
    }

    volatile uint32_t *framebuffer =
        (volatile uint32_t *)(uintptr_t)info->framebuffer_address;
    uint32_t pitch_pixels = info->framebuffer_pitch_bytes / 4u;

    for (uint32_t py = y; py < bottom; ++py) {
        for (uint32_t px = x; px < right; ++px) {
            framebuffer[(uint64_t)py * pitch_pixels + px] = color;
        }
    }
}

/*
 * Harici bootloader sözleşmesi:
 * - Kernel 64-bit long mode'da başlatılır.
 * - RDI, geçerli pinix_boot_info_t adresini içerir.
 * - Boot bilgisi ve framebuffer kernel tarafından erişilebilir durumdadır.
 */
void kernel_main(const pinix_boot_info_t *info) {
    char cpu_vendor[13];
    read_cpu_vendor(cpu_vendor);

    (void)cpu_vendor;

    if (info == (const pinix_boot_info_t *)0 ||
        info->magic != PINIX_BOOT_INFO_MAGIC ||
        info->version != PINIX_BOOT_INFO_VERSION ||
        info->struct_size < sizeof(pinix_boot_info_t)) {
        halt_forever();
    }

    if (framebuffer_is_valid(info)) {
        /* BGRX: siyah zemin ve yeşil bir örnek dikdörtgen. */
        framebuffer_clear(info, 0x00000000u);
        framebuffer_fill_rect(info, 20, 20, 200, 100, 0x0000FF00u);
    }

    /*
     * HP/Dell tespiti ve gerçek aygıt sürücüleri henüz uygulanmış değildir.
     * Firmware modu yalnızca boot_info içindeki firmware_type ile bildirilir.
     */
    halt_forever();
}
