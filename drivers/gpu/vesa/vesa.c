/*
 * vmpinix VESA Subsystem - vesa.c
 * VESA BIOS Extensions (VBE) Framebuffer Controller
 */

#define VESA_SUCCESS 0
#define VESA_ERROR -1

typedef struct {
    unsigned char signature[4];
    unsigned short version;
    unsigned int oem_string_ptr;
    unsigned int capabilities;
    unsigned int video_mode_ptr;
    unsigned short total_memory;
    unsigned short oem_software_rev;
    unsigned short oem_vendor_name_ptr;
    unsigned short oem_product_name_ptr;
    unsigned short oem_product_rev_ptr;
    unsigned char reserved[222];
    unsigned char oem_data[256];
} __attribute__((packed)) vesa_info_t;

typedef struct {
    unsigned short mode_attributes;
    unsigned char win_a_attributes;
    unsigned char win_b_attributes;
    unsigned short win_granularity;
    unsigned short win_size;
    unsigned short win_a_segment;
    unsigned short win_b_segment;
    unsigned int win_func_ptr;
    unsigned short bytes_per_scanline;
    unsigned short x_resolution;
    unsigned short y_resolution;
    unsigned char x_char_size;
    unsigned char y_char_size;
    unsigned char number_of_planes;
    unsigned char bits_per_pixel;
    unsigned char number_of_banks;
    unsigned char memory_model;
    unsigned char bank_size;
    unsigned char number_of_image_pages;
    unsigned char reserved0;
    unsigned char red_mask_size;
    unsigned char red_field_position;
    unsigned char green_mask_size;
    unsigned char green_field_position;
    unsigned char blue_mask_size;
    unsigned char blue_field_position;
    unsigned char rsvd_mask_size;
    unsigned char rsvd_field_position;
    unsigned char direct_color_mode_info;
    unsigned int phys_base_ptr;
    unsigned int off_screen_mem_off;
    unsigned short off_screen_mem_size;
    unsigned char reserved1[206];
} __attribute__((packed)) vesa_mode_info_t;

static vesa_mode_info_t current_vesa_mode_info;
static int vesa_initialized = 0;

int vesa_init(void) {
    if (vesa_initialized) {
        return VESA_SUCCESS;
    }

    current_vesa_mode_info.x_resolution = 1024;
    current_vesa_mode_info.y_resolution = 768;
    current_vesa_mode_info.bits_per_pixel = 32;
    current_vesa_mode_info.bytes_per_scanline = 1024 * 4;
    current_vesa_mode_info.phys_base_ptr = 0xFD000000;

    vesa_initialized = 1;
    return VESA_SUCCESS;
}

int vesa_set_mode(unsigned short mode_number) {
    if (!vesa_initialized) {
        vesa_init();
    }
    if (mode_number == 0x118 || mode_number == 0x143) {
        current_vesa_mode_info.x_resolution = 1024;
        current_vesa_mode_info.y_resolution = 768;
        current_vesa_mode_info.bits_per_pixel = 32;
        current_vesa_mode_info.bytes_per_scanline = 1024 * 4;
        current_vesa_mode_info.phys_base_ptr = 0xFD000000;
        return VESA_SUCCESS;
    }
    return VESA_SUCCESS;
}

vesa_mode_info_t* vesa_get_mode_info(void) {
    if (!vesa_initialized) {
        vesa_init();
    }
    return &current_vesa_mode_info;
}

void vesa_draw_pixel(unsigned int x, unsigned int y, unsigned int color) {
    if (!vesa_initialized) {
        vesa_init();
    }
    if (x >= current_vesa_mode_info.x_resolution || y >= current_vesa_mode_info.y_resolution) {
        return;
    }

    volatile unsigned int *fb = (volatile unsigned int*)current_vesa_mode_info.phys_base_ptr;
    unsigned int pitch_pixels = current_vesa_mode_info.bytes_per_scanline / 4;
    unsigned int index = (y * pitch_pixels) + x;
    fb[index] = color;
}

void vesa_clear_screen(unsigned int color) {
    if (!vesa_initialized) {
        vesa_init();
    }

    volatile unsigned int *fb = (volatile unsigned int*)current_vesa_mode_info.phys_base_ptr;
    unsigned int total_pixels = current_vesa_mode_info.x_resolution * current_vesa_mode_info.y_resolution;

    for (unsigned int i = 0; i < total_pixels; i++) {
        fb[i] = color;
    }
}

void vesa_draw_rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int color) {
    if (!vesa_initialized) {
        vesa_init();
    }

    unsigned int max_x = x + w;
    unsigned int max_y = y + h;
    unsigned int screen_w = current_vesa_mode_info.x_resolution;
    unsigned int screen_h = current_vesa_mode_info.y_resolution;

    if (max_x > screen_w) max_x = screen_w;
    if (max_y > screen_h) max_y = screen_h;

    for (unsigned int py = y; py < max_y; py++) {
        for (unsigned int px = x; px < max_x; px++) {
            vesa_draw_pixel(px, py, color);
        }
    }
}