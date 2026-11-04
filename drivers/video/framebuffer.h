#ifndef PINIX_FRAMEBUFFER_H
#define PINIX_FRAMEBUFFER_H

#include "../../include/boot_info.h"

int framebuffer_validate(const pinix_boot_info_t *info);
int framebuffer_clear(const pinix_boot_info_t *info, uint32_t color);
int framebuffer_fill_rect(const pinix_boot_info_t *info,
                          uint32_t x, uint32_t y,
                          uint32_t width, uint32_t height,
                          uint32_t color);

#endif
