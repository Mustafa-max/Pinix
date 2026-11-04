#include "kms.h"

static int kms_ready;

int kms_init(const pinix_boot_info_t *boot_info) {
    int result = drm_init(boot_info);
    kms_ready = result == DRM_SUCCESS;
    return result;
}

int kms_get_mode(drm_display_mode_t *mode) {
    if (!kms_ready) return DRM_ERROR_NO_DEVICE;
    return drm_get_mode(mode);
}

int kms_set_mode(const drm_display_mode_t *mode) {
    drm_display_mode_t current;
    if (!kms_ready) return DRM_ERROR_NO_DEVICE;
    if (mode == (const drm_display_mode_t *)0) return DRM_ERROR_INVALID_PARAM;
    if (drm_get_mode(&current) != DRM_SUCCESS) return DRM_ERROR_NO_DEVICE;

    /* The boot framebuffer is already in a firmware/loader-selected mode.
     * Refuse to claim a mode switch that this software-only path cannot do.
     */
    if (mode->width != current.width || mode->height != current.height ||
        mode->pitch_bytes != current.pitch_bytes ||
        mode->bits_per_pixel != current.bits_per_pixel) {
        return DRM_ERROR_UNSUPPORTED;
    }
    return DRM_SUCCESS;
}

int kms_present(void) {
    if (!kms_ready) return DRM_ERROR_NO_DEVICE;
    __asm__ volatile ("sfence" ::: "memory");
    return DRM_SUCCESS;
}

int kms_put_pixel(uint32_t x, uint32_t y, color_t color) {
    if (!kms_ready) return DRM_ERROR_NO_DEVICE;
    return drm_put_pixel(x, y, color);
}

int kms_fill_rect(uint32_t x, uint32_t y, uint32_t width,
                  uint32_t height, color_t color) {
    if (!kms_ready) return DRM_ERROR_NO_DEVICE;
    return drm_fill_rect(x, y, width, height, color);
}

int kms_clear(color_t color) {
    if (!kms_ready) return DRM_ERROR_NO_DEVICE;
    return drm_clear(color);
}

int kms_draw_desktop(void) {
    int result;
    if (!kms_ready) return DRM_ERROR_NO_DEVICE;
    result = drm_draw_desktop();
    if (result != DRM_SUCCESS) return result;
    return kms_present();
}
