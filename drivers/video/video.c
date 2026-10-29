#include "video.h"

extern int g_video_backend;
extern int video_select_backend(void);
void fb_set_mode(uint32_t w, uint32_t h, uint32_t pitch, uint32_t bpp, uintptr_t base);

int g_video_backend = VIDEO_BACKEND_NONE;

int gop_init(unsigned long fb_base, unsigned int w, unsigned int h, unsigned int p);
void gop_draw_pixel(unsigned int x, unsigned int y, unsigned int color);

int vesa_init(void);
void vesa_draw_pixel(unsigned int x, unsigned int y, unsigned int color);

int drm_init(void);
void drm_put_pixel(unsigned int x, unsigned int y, unsigned int color);

int kms_init(void);
void kms_draw_pixel(unsigned int x, unsigned int y, unsigned int color);

int video_init(void) {
    g_video_backend = video_select_backend();

    switch (g_video_backend) {
        case VIDEO_BACKEND_GOP:
            gop_init(0xE0000000ULL, 1024, 768, 1024 * 4);
            fb_set_mode(1024, 768, 1024 * 4, 32, 0xE0000000ULL);
            return 0;
        case VIDEO_BACKEND_VESA:
            vesa_init();
            fb_set_mode(1024, 768, 1024 * 4, 32, 0xFD000000ULL);
            return 0;
        case VIDEO_BACKEND_DRM:
            drm_init();
            fb_set_mode(1024, 768, 1024 * 4, 32, 0xE0000000ULL);
            return 0;
        case VIDEO_BACKEND_KMS:
            kms_init();
            fb_set_mode(1024, 768, 1024 * 4, 32, 0xE0000000ULL);
            return 0;
        default:
            return -1;
    }
}

void video_put_pixel(int x, int y, uint32_t color) {
    switch (g_video_backend) {
        case VIDEO_BACKEND_GOP:
            gop_draw_pixel((unsigned int)x, (unsigned int)y, color);
            break;
        case VIDEO_BACKEND_VESA:
            vesa_draw_pixel((unsigned int)x, (unsigned int)y, color);
            break;
        case VIDEO_BACKEND_DRM:
            drm_put_pixel((unsigned int)x, (unsigned int)y, color);
            break;
        case VIDEO_BACKEND_KMS:
            kms_draw_pixel((unsigned int)x, (unsigned int)y, color);
            break;
        default:
            break;
    }
}

void video_fill_rect(int x, int y, int w, int h, uint32_t color) {
    for (int yy = y; yy < y + h; ++yy) {
        for (int xx = x; xx < x + w; ++xx) {
            video_put_pixel(xx, yy, color);
        }
    }
}

void video_clear(uint32_t color) {
    video_fill_rect(0, 0, 1024, 768, color);
}
