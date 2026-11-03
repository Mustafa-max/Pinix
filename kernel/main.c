#include "boot_info.h"

static __attribute__((noreturn)) void halt_forever(void) {
    for (;;) {
        __asm__ volatile ("cli; hlt" ::: "memory");
    }
}

/*
 * Minimal kernel entry. Hardware initialization must not consume guessed
 * firmware addresses or memory maps; those are supplied through the ABI.
 * Subsystems can be initialized only after validating their inputs.
 */
void kernel_main(const pinix_boot_info_t *boot_info) {
    if (boot_info == (const pinix_boot_info_t *)0 ||
        boot_info->magic != PINIX_BOOT_INFO_MAGIC ||
        boot_info->version != PINIX_BOOT_INFO_VERSION ||
        boot_info->struct_size < sizeof(pinix_boot_info_t)) {
        halt_forever();
    }

    if (boot_info->memory_map_entries != 0 &&
        (boot_info->memory_map_address == 0 ||
         boot_info->memory_map_entry_size < sizeof(pinix_memory_region_t))) {
        halt_forever();
    }

    /* Bring-up deliberately stops here until real subsystems are implemented. */
    halt_forever();
}
