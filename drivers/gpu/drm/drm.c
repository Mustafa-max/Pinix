#include "drm.h"
#include "../../../drivers/video/framebuffer.h"

static drm_framebuffer_t primary_framebuffer;

int drm_init(const pinix_boot_info_t *boot_info) {
    if (!framebuffer_validate(boot_info)) {
        primary_framebuffer.initialized = 0;
        primary_framebuffer.pixels = (volatile pixel_bgrx32_t *)0;
        return DRM_ERROR_NO_DEVICE;
    }

    primary_framebuffer.pixels =
        (volatile pixel_bgrx32_t *)(uintptr_t)boot_info->framebuffer_address;
    primary_framebuffer.physical_or_identity_address = boot_info->framebuffer_address;
    primary_framebuffer.size_bytes = boot_info->framebuffer_size;
    primary_framebuffer.width = boot_info->framebuffer_width;
    primary_framebuffer.height = boot_info->framebuffer_height;
    primary_framebuffer.pitch_bytes = boot_info->framebuffer_pitch_bytes;
    primary_framebuffer.bits_per_pixel = boot_info->framebuffer_bpp;
    primary_framebuffer.format = boot_info->framebuffer_format;
    primary_framebuffer.initialized = 1;
    return DRM_SUCCESS;
}

int drm_is_initialized(void) {
    return primary_framebuffer.initialized != 0;
}

const drm_framebuffer_t *drm_get_framebuffer(void) {
    return drm_is_initialized() ? &primary_framebuffer :
                                  (const drm_framebuffer_t *)0;
}

int drm_get_mode(drm_display_mode_t *mode) {
    if (!drm_is_initialized() || mode == (drm_display_mode_t *)0) {
        return DRM_ERROR_INVALID_PARAM;
    }
    mode->width = primary_framebuffer.width;
    mode->height = primary_framebuffer.height;
    mode->refresh_millihz = 0; /* The boot protocol does not report refresh rate. */
    mode->pitch_bytes = primary_framebuffer.pitch_bytes;
    mode->bits_per_pixel = primary_framebuffer.bits_per_pixel;
    return DRM_SUCCESS;
}

int drm_put_pixel(uint32_t x, uint32_t y, color_t color) {
    uint64_t offset;
    pixel_bgrx32_t pixel;

    if (!drm_is_initialized() || x >= primary_framebuffer.width ||
        y >= primary_framebuffer.height) {
        return DRM_ERROR_INVALID_PARAM;
    }

    offset = (uint64_t)y * primary_framebuffer.pitch_bytes +
             (uint64_t)x * sizeof(pixel_bgrx32_t);
    if (offset > primary_framebuffer.size_bytes ||
        sizeof(pixel_bgrx32_t) > primary_framebuffer.size_bytes - offset) {
        return DRM_ERROR_INVALID_PARAM;
    }

    pixel.blue = color_blue(color);
    pixel.green = color_green(color);
    pixel.red = color_red(color);
    pixel.reserved = 0;
    primary_framebuffer.pixels[offset / sizeof(pixel)] = pixel;
    return DRM_SUCCESS;
}

int drm_fill_rect(uint32_t x, uint32_t y, uint32_t width,
                  uint32_t height, color_t color) {
    uint64_t right;
    uint64_t bottom;

    if (!drm_is_initialized()) return DRM_ERROR_NO_DEVICE;
    if (width == 0 || height == 0 || x >= primary_framebuffer.width ||
        y >= primary_framebuffer.height) return DRM_ERROR_INVALID_PARAM;

    right = (uint64_t)x + width;
    bottom = (uint64_t)y + height;
    if (right > primary_framebuffer.width) right = primary_framebuffer.width;
    if (bottom > primary_framebuffer.height) bottom = primary_framebuffer.height;

    for (uint32_t py = y; py < bottom; ++py) {
        uint64_t row_offset = (uint64_t)py * primary_framebuffer.pitch_bytes;
        for (uint32_t px = x; px < right; ++px) {
            uint64_t offset = row_offset + (uint64_t)px * sizeof(pixel_bgrx32_t);
            pixel_bgrx32_t pixel;
            if (offset > primary_framebuffer.size_bytes ||
                sizeof(pixel) > primary_framebuffer.size_bytes - offset) {
                return DRM_ERROR_INVALID_PARAM;
            }
            pixel.blue = color_blue(color);
            pixel.green = color_green(color);
            pixel.red = color_red(color);
            pixel.reserved = 0;
            primary_framebuffer.pixels[offset / sizeof(pixel)] = pixel;
        }
    }
    return DRM_SUCCESS;
}

int drm_clear(color_t color) {
    return drm_fill_rect(0, 0, primary_framebuffer.width,
                         primary_framebuffer.height, color);
}

int drm_draw_desktop(void) {
    const uint32_t panel_height = 48u;
    if (!drm_is_initialized()) return DRM_ERROR_NO_DEVICE;

    if (drm_clear(rgb_to_color(30u, 90u, 168u)) != DRM_SUCCESS) {
        return DRM_ERROR_INVALID_PARAM;
    }
    if (primary_framebuffer.height >= panel_height) {
        return drm_fill_rect(0, primary_framebuffer.height - panel_height,
                             primary_framebuffer.width, panel_height,
                             rgb_to_color(0u, 0u, 0u));
    }
    return DRM_SUCCESS;
}
