// include/boot_info.h
#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include <stdint.h>

#define BOOT_MODE_BIOS 0
#define BOOT_MODE_UEFI 1

typedef struct {
    uint64_t framebuffer_base;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint32_t framebuffer_bpp;
    uint32_t boot_mode;
} boot_info_t;

#endif