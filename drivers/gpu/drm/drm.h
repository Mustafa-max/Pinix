#ifndef PINIX_DRM_H
#define PINIX_DRM_H

#include "pixel.h"
#include "../../../include/boot_info.h"

#define DRM_SUCCESS             0
#define DRM_ERROR_NO_DEVICE    -1
#define DRM_ERROR_INVALID_PARAM -2
#define DRM_ERROR_UNSUPPORTED  -3

/* This project currently implements a software renderer to a boot framebuffer.
 * These types are not a claim of Linux DRM/GEM or native GPU driver support.
 */
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t refresh_millihz;
    uint32_t pitch_bytes;
    uint32_t bits_per_pixel;
} drm_display_mode_t;

typedef struct {
    volatile pixel_bgrx32_t *pixels;
    uint64_t physical_or_identity_address;
    uint64_t size_bytes;
    uint32_t width;
    uint32_t height;
    uint32_t pitch_bytes;
    uint32_t bits_per_pixel;
    uint32_t format;
    uint32_t initialized;
} drm_framebuffer_t;

typedef struct {
    uint32_t id;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;
    uint8_t subclass;
    uint8_t bus;
    uint8_t device;
    uint8_t function;
    uint8_t reserved[3];
} drm_device_t;

int drm_init(const pinix_boot_info_t *boot_info);
int drm_is_initialized(void);
const drm_framebuffer_t *drm_get_framebuffer(void);
int drm_get_mode(drm_display_mode_t *mode);
int drm_put_pixel(uint32_t x, uint32_t y, color_t color);
int drm_fill_rect(uint32_t x, uint32_t y, uint32_t width,
                  uint32_t height, color_t color);
int drm_clear(color_t color);
int drm_draw_desktop(void);

#endif
