#include "gop.h"

typedef struct {
    void *framebuffer_base;
    unsigned long framebuffer_size;
    unsigned int width;
    unsigned int height;
    unsigned int pitch;
    unsigned int bytes_per_pixel;
    int active;
    gop_mode_info_t mode_info;
} uefi_gop_driver_t;

static uefi_gop_driver_t gop_state;

int gop_init(unsigned long fb_base, unsigned int width, unsigned int height, unsigned int pitch) {
    if (!fb_base || !width || !height || !pitch) {
        return GOP_ERROR;
    }

    gop_state.framebuffer_base = (void *)fb_base;
    gop_state.width = width;
    gop_state.height = height;
    gop_state.pitch = pitch;
    gop_state.bytes_per_pixel = 4;
    gop_state.framebuffer_size = (unsigned long)height * pitch;
    gop_state.active = 1;

    gop_state.mode_info.horizontal_resolution = width;
    gop_state.mode_info.vertical_resolution = height;
    gop_state.mode_info.pixels_per_scan_line = pitch / 4;

    return GOP_SUCCESS;
}

int gop_set_mode(unsigned int mode_number) {
    (void)mode_number;
    return gop_state.active ? GOP_SUCCESS : GOP_ERROR;
}

gop_mode_info_t *gop_get_mode_info(void) {
    return gop_state.active ? &gop_state.mode_info : 0;
}

void gop_draw_pixel(unsigned int x, unsigned int y, unsigned int color) {
    if (!gop_state.active || x >= gop_state.width || y >= gop_state.height) {
        return;
    }

    volatile unsigned int *fb = (volatile unsigned int *)gop_state.framebuffer_base;
    unsigned int index = (y * (gop_state.pitch / 4)) + x;
    fb[index] = color;
}

void gop_clear_screen(unsigned int color) {
    if (!gop_state.active || !gop_state.framebuffer_base) {
        return;
    }

    volatile unsigned int *fb = (volatile unsigned int *)gop_state.framebuffer_base;

    for (unsigned int y = 0; y < gop_state.height; ++y) {
        for (unsigned int x = 0; x < gop_state.width; ++x) {
            fb[(y * (gop_state.pitch / 4)) + x] = color;
        }
    }
}

void gop_draw_rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int color) {
    unsigned int max_x = x + w;
    unsigned int max_y = y + h;

    if (max_x > gop_state.width) max_x = gop_state.width;
    if (max_y > gop_state.height) max_y = gop_state.height;

    for (unsigned int py = y; py < max_y; ++py) {
        for (unsigned int px = x; px < max_x; ++px) {
            gop_draw_pixel(px, py, color);
        }
    }
}