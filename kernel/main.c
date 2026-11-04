#include "boot_info.h"
#include "../drivers/serial/serial.h"
#include "../drivers/video/framebuffer.h"
#include "../drivers/check/pci.h"

#define MULTIBOOT2_BOOT_MAGIC 0x36D76289u
#define MB2_TAG_END 0u
#define MB2_TAG_MMAP 6u
#define MB2_TAG_FRAMEBUFFER 8u
#define MB2_FRAMEBUFFER_TYPE_RGB 1u
#define MB2_INFO_MAX_SIZE (16u * 1024u * 1024u)

struct mb2_info_header {
    uint32_t total_size;
    uint32_t reserved;
} __attribute__((packed));

struct mb2_tag {
    uint32_t type;
    uint32_t size;
} __attribute__((packed));

struct mb2_framebuffer_tag {
    uint32_t type;
    uint32_t size;
    uint64_t address;
    uint32_t pitch;
    uint32_t width;
    uint32_t height;
    uint8_t bpp;
    uint8_t framebuffer_type;
    uint16_t reserved;
    uint8_t red_position;
    uint8_t red_mask_size;
    uint8_t green_position;
    uint8_t green_mask_size;
    uint8_t blue_position;
    uint8_t blue_mask_size;
} __attribute__((packed));

static __attribute__((noreturn)) void halt_forever(void) {
    for (;;) {
        __asm__ volatile ("cli; hlt" ::: "memory");
    }
}

static int parse_multiboot2(uint32_t magic, uint32_t info_address,
                            pinix_boot_info_t *boot_info) {
    const struct mb2_info_header *header;
    const uint8_t *base;
    uint32_t offset;
    int saw_end = 0;

    if (magic != MULTIBOOT2_BOOT_MAGIC || info_address == 0 ||
        boot_info == (pinix_boot_info_t *)0) {
        return -1;
    }

    header = (const struct mb2_info_header *)(uintptr_t)info_address;
    if (header->total_size < sizeof(*header) + sizeof(struct mb2_tag) ||
        header->total_size > MB2_INFO_MAX_SIZE) {
        return -1;
    }

    base = (const uint8_t *)header;
    offset = sizeof(*header);

    while (offset <= header->total_size - sizeof(struct mb2_tag)) {
        const struct mb2_tag *tag =
            (const struct mb2_tag *)(const void *)(base + offset);
        uint32_t aligned_size;

        if (tag->size < sizeof(*tag) || tag->size > header->total_size - offset) {
            return -1;
        }

        if (tag->type == MB2_TAG_END) {
            if (tag->size != sizeof(*tag)) return -1;
            saw_end = 1;
            break;
        }

        if (tag->type == MB2_TAG_MMAP) {
            /* Validate the memory-map tag structure before any later consumer. */
            if (tag->size < 16u) return -1;
            const uint32_t entry_size =
                *(const uint32_t *)(const void *)(base + offset + 8u);
            if (entry_size < 24u || (tag->size - 16u) % entry_size != 0u) {
                return -1;
            }
        } else if (tag->type == MB2_TAG_FRAMEBUFFER) {
            if (tag->size >= sizeof(struct mb2_framebuffer_tag)) {
                const struct mb2_framebuffer_tag *fb =
                    (const struct mb2_framebuffer_tag *)(const void *)tag;
                if (fb->framebuffer_type == MB2_FRAMEBUFFER_TYPE_RGB &&
                    fb->bpp == 32u && fb->red_position == 16u &&
                    fb->green_position == 8u && fb->blue_position == 0u &&
                    fb->address != 0 && fb->height != 0 &&
                    fb->pitch <= 0xFFFFFFFFu / fb->height) {
                    boot_info->framebuffer_address = fb->address;
                    boot_info->framebuffer_size =
                        (uint64_t)fb->pitch * fb->height;
                    boot_info->framebuffer_width = fb->width;
                    boot_info->framebuffer_height = fb->height;
                    boot_info->framebuffer_pitch_bytes = fb->pitch;
                    boot_info->framebuffer_bpp = fb->bpp;
                    boot_info->framebuffer_format = PINIX_FB_FORMAT_BGRX;
                }
            }
        }

        aligned_size = (tag->size + 7u) & ~7u;
        if (aligned_size < tag->size || aligned_size > header->total_size - offset) {
            return -1;
        }
        offset += aligned_size;
    }

    return saw_end ? 0 : -1;
}

static void draw_boot_desktop(const pinix_boot_info_t *info) {
    if (!framebuffer_validate(info)) {
        serial_write_string("Pinix: no supported RGB framebuffer\n");
        return;
    }

    /* Firmware-selected mode is used; 1920x1080 is only a request to GRUB. */
    (void)framebuffer_clear(info, 0x00A85A1Eu);
    if (info->framebuffer_height >= 48u) {
        (void)framebuffer_fill_rect(info, 0,
            info->framebuffer_height - 48u,
            info->framebuffer_width, 48u, 0x00000000u);
    }
    serial_write_string("Pinix: Multiboot2 framebuffer ready\n");
}

static void log_cpu_vendor(void) {
    uint32_t eax, ebx, ecx, edx;
    char vendor[13];

    __asm__ volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(0), "c"(0)
    );
    (void)eax;

    vendor[0] = (char)ebx;
    vendor[1] = (char)(ebx >> 8);
    vendor[2] = (char)(ebx >> 16);
    vendor[3] = (char)(ebx >> 24);
    vendor[4] = (char)edx;
    vendor[5] = (char)(edx >> 8);
    vendor[6] = (char)(edx >> 16);
    vendor[7] = (char)(edx >> 24);
    vendor[8] = (char)ecx;
    vendor[9] = (char)(ecx >> 8);
    vendor[10] = (char)(ecx >> 16);
    vendor[11] = (char)(ecx >> 24);
    vendor[12] = '\0';

    serial_write_string("Pinix: CPU vendor: ");
    serial_write_string(vendor);
    serial_write_string("\n");
}

static void log_pci_inventory(void) {
    uint32_t count = pinix_pci_scan();
    const pinix_pci_device_t *list = pinix_pci_get_devices(&count);

    serial_write_string("Pinix: PCI devices: ");
    serial_write_hex64(count);
    serial_write_string("\n");

    for (uint32_t i = 0; i < count; ++i) {
        serial_write_string("PCI ");
        serial_write_hex64(list[i].bus);
        serial_write_char(':');
        serial_write_hex64(list[i].device);
        serial_write_char('.');
        serial_write_hex64(list[i].function);
        serial_write_string(" vendor=");
        serial_write_hex64(list[i].vendor_id);
        serial_write_string(" device=");
        serial_write_hex64(list[i].device_id);
        serial_write_string(" class=");
        serial_write_hex64(list[i].class_code);
        serial_write_char(':');
        serial_write_hex64(list[i].subclass);
        serial_write_char('\n');
    }
}

void kernel_main(uint32_t multiboot_magic, uint32_t multiboot_info_address) {
    pinix_boot_info_t boot_info = {0};

    (void)serial_init();
    serial_write_string("Pinix x86_64 Multiboot2 kernel\n");
    log_cpu_vendor();

    if (parse_multiboot2(multiboot_magic, multiboot_info_address,
                         &boot_info) != 0) {
        serial_write_string("Pinix: invalid Multiboot2 info; halted\n");
        halt_forever();
    }

    draw_boot_desktop(&boot_info);
    log_pci_inventory();

    /* No native GPU modesetting or interrupt-driven input is claimed here. */
    halt_forever();
}
