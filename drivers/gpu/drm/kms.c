#include "kms.h"

static drm_framebuffer_t active_fb;

void kms_set_fb_address(void *addr, unsigned int pitch, unsigned int width, unsigned int height) {
    active_fb.buffer = addr;
    active_fb.pitch = pitch;
    active_fb.width = width;
    active_fb.height = height;
    active_fb.bpp = 32;
}

void kms_clear_screen(color_t color) {
    if (!active_fb.buffer || !active_fb.width || !active_fb.height) {
        return;
    }

    volatile unsigned int *fb = (volatile unsigned int *)active_fb.buffer;
    unsigned int pitch_px = active_fb.pitch / 4;

    for (unsigned int y = 0; y < active_fb.height; ++y) {
        for (unsigned int x = 0; x < active_fb.width; ++x) {
            fb[(y * pitch_px) + x] = (unsigned int)color;
        }
    }
}

void kms_draw_rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, color_t color) {
    if (!active_fb.buffer || !active_fb.width || !active_fb.height) {
        return;
    }

    unsigned int max_x = x + w;
    unsigned int max_y = y + h;

    if (max_x > active_fb.width) max_x = active_fb.width;
    if (max_y > active_fb.height) max_y = active_fb.height;

    volatile unsigned int *fb = (volatile unsigned int *)active_fb.buffer;
    unsigned int pitch_px = active_fb.pitch / 4;

    for (unsigned int py = y; py < max_y; ++py) {
        for (unsigned int px = x; px < max_x; ++px) {
            fb[(py * pitch_px) + px] = (unsigned int)color;
        }
    }
}