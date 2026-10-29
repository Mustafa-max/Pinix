/*
 * Pinix-Kernel KMS Subsystem - kms.h
 * Kernel Mode Setting Header and Prototypes
 */

#ifndef KMS_H
#define KMS_H

#include "drm.h"
#include "pixel.h"

int kms_init(void);
int kms_set_mode(drm_display_mode_t *mode);
drm_framebuffer_t* kms_create_framebuffer(unsigned int width, unsigned int height, unsigned int bpp);
void kms_set_fb_address(void *addr, unsigned int pitch, unsigned int width, unsigned int height);
void kms_destroy_framebuffer(drm_framebuffer_t *fb);
void kms_swap_buffers(drm_framebuffer_t *fb);
int kms_add_property(unsigned int fb_id, const char *name, unsigned long value);
void kms_clear_screen(color_t color);
void kms_draw_pixel(unsigned int x, unsigned int y, color_t color);
void kms_draw_rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, color_t color);
int kms_get_connector_status(unsigned int connector_id);
int kms_set_crtc_mode(unsigned int crtc_id, drm_display_mode_t *mode);

#endif