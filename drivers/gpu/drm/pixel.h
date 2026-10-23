/*
 * Pinix-Kernel Pixel Subsystem - pixel.h
 * Color and Pixel Representation Structures
 */

#ifndef PIXEL_H
#define PIXEL_H

typedef unsigned int color_t;

typedef struct {
    unsigned char b;
    unsigned char g;
    unsigned char r;
    unsigned char a;
} __attribute__((packed)) pixel_rgb32_t;

static inline color_t rgb_to_color(unsigned char r, unsigned char g, unsigned char b) {
    return ((color_t)r << 16) | ((color_t)g << 8) | (color_t)b;
}

static inline color_t rgba_to_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    return ((color_t)a << 24) | ((color_t)r << 16) | ((color_t)g << 8) | (color_t)b;
}

#endif