#ifndef PINIX_PIXEL_H
#define PINIX_PIXEL_H

#include "../../../include/stdint.h"

/* Public colors are 0x00RRGGBB. In BGRX framebuffer memory the bytes become
 * B, G, R, X on little-endian x86-64, as required by the accepted boot format.
 */
typedef uint32_t color_t;

typedef struct {
    uint8_t blue;
    uint8_t green;
    uint8_t red;
    uint8_t reserved;
} __attribute__((packed)) pixel_bgrx32_t;

static inline color_t rgb_to_color(uint8_t red, uint8_t green, uint8_t blue) {
    return ((color_t)red << 16) | ((color_t)green << 8) | (color_t)blue;
}

static inline uint8_t color_red(color_t color) {
    return (uint8_t)((color >> 16) & 0xFFu);
}

static inline uint8_t color_green(color_t color) {
    return (uint8_t)((color >> 8) & 0xFFu);
}

static inline uint8_t color_blue(color_t color) {
    return (uint8_t)(color & 0xFFu);
}

static inline color_t color_blend(color_t foreground, color_t background,
                                  uint8_t alpha) {
    uint32_t inverse = 255u - alpha;
    uint32_t red = ((uint32_t)color_red(foreground) * alpha +
                    (uint32_t)color_red(background) * inverse) / 255u;
    uint32_t green = ((uint32_t)color_green(foreground) * alpha +
                      (uint32_t)color_green(background) * inverse) / 255u;
    uint32_t blue = ((uint32_t)color_blue(foreground) * alpha +
                     (uint32_t)color_blue(background) * inverse) / 255u;
    return rgb_to_color((uint8_t)red, (uint8_t)green, (uint8_t)blue);
}

#endif
