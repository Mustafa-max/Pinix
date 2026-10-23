/*
 * Pinix-Kernel GOP Subsystem - gop.h
 * Graphics Output Protocol Header for UEFI
 */

#ifndef GOP_H
#define GOP_H

#define GOP_SUCCESS 0
#define GOP_ERROR -1

typedef struct {
    unsigned int red_mask;
    unsigned int green_mask;
    unsigned int blue_mask;
    unsigned int reserved_mask;
} gop_pixel_bitmask_t;

typedef enum {
    PixelRedGreenBlueReserved8BitPerColor,
    PixelBlueGreenRedReserved8BitPerColor,
    PixelBitMask,
    PixelBltOnly,
    PixelFormatMax
} gop_graphics_pixel_format_t;

typedef struct {
    unsigned int version;
    unsigned int horizontal_resolution;
    unsigned int vertical_resolution;
    gop_graphics_pixel_format_t pixel_format;
    gop_pixel_bitmask_t pixel_information;
    unsigned int pixels_per_scan_line;
} gop_mode_info_t;

typedef struct {
    unsigned int max_mode;
    unsigned int mode;
    gop_mode_info_t *info;
    unsigned long size_of_info;
    unsigned long frame_buffer_base;
    unsigned long frame_buffer_size;
} gop_mode_t;

int gop_init(unsigned long fb_base, unsigned int width, unsigned int height, unsigned int pitch);
int gop_set_mode(unsigned int mode_number);
gop_mode_info_t* gop_get_mode_info(void);
void gop_draw_pixel(unsigned int x, unsigned int y, unsigned int color);
void gop_clear_screen(unsigned int color);
void gop_draw_rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int color);

#endif