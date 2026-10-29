#include "video.h"

static video_fb_t g_fb = {
    .width = 1024,
    .height = 768,
    .pitch = 1024 * 4,
    .bpp = 32,
    .base = 0xE0000000ULL,
    .initialized = 0
};

void fb_set_mode(uint32_t w, uint32_t h, uint32_t pitch, uint32_t bpp, uintptr_t base) {
    g_fb.width = w;
    g_fb.height = h;
    g_fb.pitch = pitch;
    g_fb.bpp = bpp;
    g_fb.base = base;
    g_fb.initialized = 1;
}

video_fb_t *video_get_fb(void) {
    return &g_fb;
}
