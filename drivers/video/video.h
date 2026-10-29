#ifndef VIDEO_H
#define VIDEO_H

#include <stdint.h>

#define VIDEO_BACKEND_NONE 0
#define VIDEO_BACKEND_GOP  1
#define VIDEO_BACKEND_VESA 2
#define VIDEO_BACKEND_DRM  3
#define VIDEO_BACKEND_KMS 4

typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint32_t bpp;
    uintptr_t base;
    int initialized;
} video_fb_t;

int video_init(void);
void video_put_pixel(int x, int y, uint32_t color);
void video_fill_rect(int x, int y, int w, int h, uint32_t color);
void video_clear(uint32_t color);
video_fb_t *video_get_fb(void);

#endif
