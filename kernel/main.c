#include "boot_info.h"
#include "../drivers/serial/serial.h"
#include "../drivers/check/pci.h"
#include "../drivers/gpu/drm/kms.h"

#define MULTIBOOT2_BOOT_MAGIC 0x36D76289u
#define MB2_TAG_END 0u
#define MB2_TAG_FRAMEBUFFER 8u
#define MB2_FRAMEBUFFER_RGB 1u
#define MB2_INFO_LIMIT (16u * 1024u * 1024u)

struct mb2_info_header { uint32_t total_size, reserved; } __attribute__((packed));
struct mb2_tag { uint32_t type, size; } __attribute__((packed));
struct mb2_fb_tag {
    uint32_t type, size;
    uint64_t address;
    uint32_t pitch, width, height;
    uint8_t bpp, framebuffer_type;
    uint16_t reserved;
    uint8_t red_position, red_mask_size;
    uint8_t green_position, green_mask_size;
    uint8_t blue_position, blue_mask_size;
} __attribute__((packed));

static __attribute__((noreturn)) void halt_forever(void) {
    for (;;) __asm__ volatile ("cli; hlt" ::: "memory");
}

static int parse_multiboot2(uint32_t magic, uint32_t info_address,
                            pinix_boot_info_t *out) {
    const struct mb2_info_header *header;
    const uint8_t *base;
    uint32_t offset;
    int saw_end = 0;

    if (magic != MULTIBOOT2_BOOT_MAGIC || info_address == 0 || out == 0) return -1;
    header = (const struct mb2_info_header *)(uintptr_t)info_address;
    if ((uint64_t)info_address + sizeof(*header) > 0x100000000ULL ||
        header->total_size < sizeof(*header) + sizeof(struct mb2_tag) ||
        header->total_size > MB2_INFO_LIMIT ||
        (uint64_t)info_address + header->total_size > 0x100000000ULL) return -1;

    base = (const uint8_t *)header;
    offset = sizeof(*header);
    while (offset <= header->total_size - sizeof(struct mb2_tag)) {
        const struct mb2_tag *tag =
            (const struct mb2_tag *)(const void *)(base + offset);
        uint32_t aligned_size;

        if (tag->size < sizeof(*tag) || tag->size > header->total_size - offset) return -1;
        if (tag->type == MB2_TAG_END) {
            if (tag->size != sizeof(*tag)) return -1;
            saw_end = 1;
            break;
        }
        if (tag->type == MB2_TAG_FRAMEBUFFER && tag->size >= sizeof(struct mb2_fb_tag)) {
            const struct mb2_fb_tag *fb = (const struct mb2_fb_tag *)(const void *)tag;
            uint64_t bytes = (uint64_t)fb->pitch * fb->height;
            if (fb->framebuffer_type == MB2_FRAMEBUFFER_RGB && fb->bpp == 32u &&
                fb->red_position == 16u && fb->green_position == 8u &&
                fb->blue_position == 0u && fb->address != 0 && fb->width != 0 &&
                fb->height != 0 && (uint64_t)fb->pitch >= (uint64_t)fb->width * 4u &&
                bytes / fb->height == fb->pitch && fb->address <= 0xFFFFFFFFULL &&
                bytes <= 0x100000000ULL - fb->address) {
                out->framebuffer_address = fb->address;
                out->framebuffer_size = bytes;
                out->framebuffer_width = fb->width;
                out->framebuffer_height = fb->height;
                out->framebuffer_pitch_bytes = fb->pitch;
                out->framebuffer_bpp = fb->bpp;
                out->framebuffer_format = PINIX_FB_FORMAT_BGRX;
            }
        }
        aligned_size = (tag->size + 7u) & ~7u;
        if (aligned_size < tag->size || aligned_size > header->total_size - offset) return -1;
        offset += aligned_size;
    }
    return saw_end ? 0 : -1;
}

static void log_cpu(void) {
    uint32_t eax, ebx, ecx, edx;
    char vendor[13];
    __asm__ volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(0), "c"(0));
    (void)eax;
    vendor[0] = (char)ebx; vendor[1] = (char)(ebx >> 8);
    vendor[2] = (char)(ebx >> 16); vendor[3] = (char)(ebx >> 24);
    vendor[4] = (char)edx; vendor[5] = (char)(edx >> 8);
    vendor[6] = (char)(edx >> 16); vendor[7] = (char)(edx >> 24);
    vendor[8] = (char)ecx; vendor[9] = (char)(ecx >> 8);
    vendor[10] = (char)(ecx >> 16); vendor[11] = (char)(ecx >> 24); vendor[12] = 0;
    serial_write_string("Pinix CPU vendor: ");
    serial_write_string(vendor);
    serial_write_char('\n');
}

static void log_pci_inventory(void) {
    uint32_t count = pinix_pci_scan();
    const pinix_pci_device_t *list = pinix_pci_get_devices(&count);
    serial_write_string("PCI functions: ");
    serial_write_hex64(count);
    serial_write_char('\n');
    for (uint32_t i = 0; i < count; ++i) {
        serial_write_string("PCI "); serial_write_hex64(list[i].bus);
        serial_write_char(':'); serial_write_hex64(list[i].device);
        serial_write_char('.'); serial_write_hex64(list[i].function);
        serial_write_string(" VID="); serial_write_hex64(list[i].vendor_id);
        serial_write_string(" DID="); serial_write_hex64(list[i].device_id);
        serial_write_char('\n');
    }
}

void kernel_main(uint32_t magic, uint32_t mbi_address) {
    pinix_boot_info_t boot_info = {0};

    (void)serial_init();
    serial_write_string("Pinix Multiboot2 x86_64 kernel\n");
    log_cpu();

    if (parse_multiboot2(magic, mbi_address, &boot_info) != 0) {
        serial_write_string("Pinix: invalid Multiboot2 information; halted\n");
        halt_forever();
    }

    if (kms_init(&boot_info) == DRM_SUCCESS) {
        drm_display_mode_t mode;
        if (kms_get_mode(&mode) == DRM_SUCCESS) {
            serial_write_string("Pinix display mode: ");
            serial_write_hex64(mode.width);
            serial_write_char('x');
            serial_write_hex64(mode.height);
            serial_write_char('\n');
        }
        (void)kms_draw_desktop();
        serial_write_string("Pinix software desktop background and bottom panel drawn\n");
    } else {
        serial_write_string("Pinix: no supported boot framebuffer\n");
    }

    log_pci_inventory();
    serial_write_string("Native GPU mode-setting is not implemented; using boot framebuffer only\n");
    halt_forever();
}
