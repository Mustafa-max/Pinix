#include "framebuffer.h"

static int framebuffer_layout_valid(const pinix_boot_info_t *info) {
    uint64_t row_bytes;
    uint64_t rows_before_last;
    uint64_t required_size;
    const uint64_t max_u64 = ~(uint64_t)0;

    if (info == (const pinix_boot_info_t *)0 ||
        info->framebuffer_address == 0 ||
        info->framebuffer_size == 0 ||
        info->framebuffer_width == 0 ||
        info->framebuffer_height == 0 ||
        info->framebuffer_bpp != 32 ||
        info->framebuffer_format != PINIX_FB_FORMAT_BGRX) {
        return 0;
    }

    row_bytes = (uint64_t)info->framebuffer_width * 4u;
    if (info->framebuffer_pitch_bytes < row_bytes) {
        return 0;
    }

    rows_before_last = (uint64_t)info->framebuffer_height - 1u;
    if (rows_before_last != 0 &&
        info->framebuffer_pitch_bytes > (max_u64 - row_bytes) / rows_before_last) {
        return 0;
    }

    required_size = rows_before_last * info->framebuffer_pitch_bytes + row_bytes;
    if (required_size > info->framebuffer_size ||
        info->framebuffer_address > max_u64 - required_size) {
        return 0;
    }
    return 1;
}

int framebuffer_validate(const pinix_boot_info_t *info) {
    return framebuffer_layout_valid(info);
}

int framebuffer_clear(const pinix_boot_info_t *info, uint32_t color) {
    if (!framebuffer_layout_valid(info)) {
        return -1;
    }

    volatile uint32_t *pixels =
        (volatile uint32_t *)(uintptr_t)info->framebuffer_address;
    const uint32_t pitch_pixels = info->framebuffer_pitch_bytes / 4u;

    for (uint32_t y = 0; y < info->framebuffer_height; ++y) {
        for (uint32_t x = 0; x < info->framebuffer_width; ++x) {
            pixels[(uint64_t)y * pitch_pixels + x] = color;
        }
    }
    return 0;
}

int framebuffer_fill_rect(const pinix_boot_info_t *info,
                          uint32_t x, uint32_t y,
                          uint32_t width, uint32_t height,
                          uint32_t color) {
    uint64_t right;
    uint64_t bottom;
    volatile uint32_t *pixels;
    uint32_t pitch_pixels;

    if (!framebuffer_layout_valid(info) ||
        x >= info->framebuffer_width || y >= info->framebuffer_height) {
        return -1;
    }

    right = (uint64_t)x + width;
    bottom = (uint64_t)y + height;
    if (right > info->framebuffer_width) right = info->framebuffer_width;
    if (bottom > info->framebuffer_height) bottom = info->framebuffer_height;

    pixels = (volatile uint32_t *)(uintptr_t)info->framebuffer_address;
    pitch_pixels = info->framebuffer_pitch_bytes / 4u;
    for (uint32_t py = y; py < bottom; ++py) {
        for (uint32_t px = x; px < right; ++px) {
            pixels[(uint64_t)py * pitch_pixels + px] = color;
        }
    }
    return 0;
}
