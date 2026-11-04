#ifndef PINIX_BOOT_INFO_H
#define PINIX_BOOT_INFO_H

#include "stdint.h"

#define PINIX_BOOT_INFO_MAGIC   0x50494E4958424931ULL /* "PINIXBI1" */
#define PINIX_BOOT_INFO_VERSION 1u

#define PINIX_BOOT_FIRMWARE_UNKNOWN 0u
#define PINIX_BOOT_FIRMWARE_BIOS    1u
#define PINIX_BOOT_FIRMWARE_UEFI    2u

#define PINIX_MEMORY_RESERVED 0u
#define PINIX_MEMORY_USABLE   1u
#define PINIX_MEMORY_ACPI     2u
#define PINIX_MEMORY_MMIO     3u
#define PINIX_MEMORY_BAD      4u

/* 32-bit pixel storage: bytes B, G, R, X in increasing address order. */
#define PINIX_FB_FORMAT_BGRX 1u

/* Framebuffer address is a kernel-accessible virtual address, not just a PA. */
typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t reserved;
} pinix_memory_region_t;

typedef struct {
    uint64_t magic;
    uint32_t version;
    uint32_t struct_size;

    uint32_t firmware_type;
    uint32_t flags;

    uint64_t memory_map_address;
    uint64_t memory_map_entries;
    uint64_t memory_map_entry_size;

    uint64_t framebuffer_address;
    uint64_t framebuffer_size;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint32_t framebuffer_pitch_bytes;
    uint32_t framebuffer_bpp;
    uint32_t framebuffer_format;
    uint32_t reserved;
} pinix_boot_info_t;

#endif
