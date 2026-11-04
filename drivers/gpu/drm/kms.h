#ifndef PINIX_KMS_H
#define PINIX_KMS_H

#include "drm.h"

/* KMS here is a small display-pipeline helper over the loader's framebuffer.
 * It does not program GPU registers or perform native hardware mode-setting.
 */
int kms_init(const pinix_boot_info_t *boot_info);
int kms_get_mode(drm_display_mode_t *mode);
int kms_set_mode(const drm_display_mode_t *mode);
int kms_present(void);
int kms_put_pixel(uint32_t x, uint32_t y, color_t color);
int kms_fill_rect(uint32_t x, uint32_t y, uint32_t width,
                  uint32_t height, color_t color);
int kms_clear(color_t color);
int kms_draw_desktop(void);

#endif
