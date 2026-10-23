#include "../../include/driver.h"

#define FB_GREEN 0x00FF00
#define FB_YELLOW 0xFFFF00

static uint32_t *g_fb = 0;
static uint32_t g_width = 1024;
static uint32_t g_height = 768;
static uint32_t g_pitch = 1024 * 4;

static void fb_fill(uint32_t *fb, uint32_t width, uint32_t height, uint32_t pitch, uint32_t color) {
    uint32_t pitch_px = pitch / 4;
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            fb[(y * pitch_px) + x] = color;
        }
    }
}

static void fb_rect(uint32_t *fb, uint32_t width, uint32_t height, uint32_t pitch,
                    uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    uint32_t pitch_px = pitch / 4;

    if (x >= width || y >= height) return;
    uint32_t max_x = x + w;
    uint32_t max_y = y + h;
    if (max_x > width) max_x = width;
    if (max_y > height) max_y = height;

    for (uint32_t py = y; py < max_y; ++py) {
        for (uint32_t px = x; px < max_x; ++px) {
            fb[(py * pitch_px) + px] = color;
        }
    }
}

static int gpu_probe(void) {
    return DRIVER_OK;
}

static int gpu_init(void) {
    g_fb = (uint32_t *)0xE0000000ULL;
    if (!g_fb) return DRIVER_ERR;

    fb_fill(g_fb, g_width, g_height, g_pitch, FB_GREEN);
    fb_rect(g_fb, g_width, g_height, g_pitch, g_width - 160, 20, 120, 120, FB_YELLOW);

    return DRIVER_OK;
}

static driver_t gpu_driver = {
    .name = "gpu",
    .type = DRIVER_TYPE_GPU,
    .probe = gpu_probe,
    .init = gpu_init,
    .next = 0
};

int gpu_driver_register(void) {
    return driver_register(&gpu_driver);
}