#include "boot_info.h"
#include "../drivers/serial/serial.h"
#include "../drivers/video/framebuffer.h"
#include "../drivers/input/ps2.h"

static __attribute__((noreturn)) void halt_forever(void) {
    for (;;) {
        __asm__ volatile ("cli; hlt" ::: "memory");
    }
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

    serial_write_string("Pinix: CPUID vendor: ");
    serial_write_string(vendor);
    serial_write_string("\n");
}

void kernel_main(const pinix_boot_info_t *boot_info) {
    (void)serial_init();
    serial_write_string("Pinix x86_64 kernel starting\n");
    log_cpu_vendor();

    if (boot_info == (const pinix_boot_info_t *)0 ||
        boot_info->magic != PINIX_BOOT_INFO_MAGIC ||
        boot_info->version != PINIX_BOOT_INFO_VERSION ||
        boot_info->struct_size < sizeof(pinix_boot_info_t)) {
        serial_write_string("Pinix: invalid boot information; halted\n");
        halt_forever();
    }

    if (boot_info->memory_map_entries != 0 &&
        (boot_info->memory_map_address == 0 ||
         boot_info->memory_map_entry_size < sizeof(pinix_memory_region_t))) {
        serial_write_string("Pinix: invalid memory map; halted\n");
        halt_forever();
    }

    if (boot_info->firmware_type == PINIX_BOOT_FIRMWARE_UEFI) {
        serial_write_string("Pinix: booted by a UEFI-mode external loader\n");
    } else if (boot_info->firmware_type == PINIX_BOOT_FIRMWARE_BIOS) {
        serial_write_string("Pinix: booted by a BIOS-mode external loader\n");
    } else {
        serial_write_string("Pinix: firmware mode not specified\n");
    }

    if (framebuffer_validate(boot_info)) {
        (void)framebuffer_clear(boot_info, 0x00000000u);
        (void)framebuffer_fill_rect(boot_info, 20, 20, 200, 100, 0x0000FF00u);
        serial_write_string("Pinix: boot framebuffer available\n");
    } else {
        serial_write_string("Pinix: no supported boot framebuffer\n");
    }

    if (ps2_kbd_init() == PS2_OK) {
        serial_write_string("Pinix: PS/2 keyboard initialized (polling)\n");
    } else {
        serial_write_string("Pinix: PS/2 keyboard unavailable\n");
    }

    if (ps2_mouse_init() == PS2_OK) {
        serial_write_string("Pinix: PS/2 mouse initialized (polling)\n");
    } else {
        serial_write_string("Pinix: PS/2 mouse unavailable\n");
    }

    /* Interrupt handling, USB/xHCI, storage, and memory management remain TODO. */
    halt_forever();
}
