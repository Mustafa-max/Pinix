/*
 * vmpinix VGA Subsystem - vga.c
 * Standard VGA Text and Graphics Mode Controller
 */

#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA  0x3D5
#define VGA_GC_INDEX   0x3CE
#define VGA_GC_DATA    0x3CF
#define VGA_AC_INDEX   0x3C0
#define VGA_AC_WRITE   0x3C0
#define VGA_AC_READ    0x3C1
#define VGA_SEQ_INDEX  0x3C4
#define VGA_SEQ_DATA   0x3C5
#define VGA_MISC_READ  0x3CC
#define VGA_MISC_WRITE 0x3C2

#define VGA_WIDTH  80
#define VGA_HEIGHT 25

typedef struct {
    unsigned char misc;
    unsigned char seq[5];
    unsigned char crtc[25];
    unsigned char gc[9];
    unsigned char ac[21];
} vga_registers_t;

static unsigned short *vga_text_buffer = (unsigned short*)0xB8000;
static unsigned char vga_current_color = 0x0F; // Beyaz üstüne siyah
static unsigned int vga_cursor_x = 0;
static unsigned int vga_cursor_y = 0;

static inline void vga_outb(unsigned short port, unsigned char val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline unsigned char vga_inb(unsigned short port) {
    unsigned char ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void vga_init_text_mode(void) {
    vga_cursor_x = 0;
    vga_cursor_y = 0;
    vga_current_color = 0x0F;
    
    for (unsigned int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_text_buffer[i] = (unsigned short)(' ' | (vga_current_color << 8));
    }
}

void vga_set_cursor(unsigned int x, unsigned int y) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) {
        return;
    }
    vga_cursor_x = x;
    vga_cursor_y = y;
    unsigned short pos = y * VGA_WIDTH + x;

    vga_outb(VGA_CRTC_INDEX, 0x0F);
    vga_outb(VGA_CRTC_DATA, (unsigned char)(pos & 0xFF));
    vga_outb(VGA_CRTC_INDEX, 0x0E);
    vga_outb(VGA_CRTC_DATA, (unsigned char)((pos >> 8) & 0xFF));
}

void vga_clear_screen(void) {
    for (unsigned int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_text_buffer[i] = (unsigned short)(' ' | (vga_current_color << 8));
    }
    vga_set_cursor(0, 0);
}

void vga_scroll_up(void) {
    for (unsigned int i = 0; i < VGA_WIDTH * (VGA_HEIGHT - 1); i++) {
        vga_text_buffer[i] = vga_text_buffer[i + VGA_WIDTH];
    }
    for (unsigned int i = VGA_WIDTH * (VGA_HEIGHT - 1); i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_text_buffer[i] = (unsigned short)(' ' | (vga_current_color << 8));
    }
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_cursor_x = 0;
        vga_cursor_y++;
    } else if (c == '\r') {
        vga_cursor_x = 0;
    } else if (c == '\b') {
        if (vga_cursor_x > 0) {
            vga_cursor_x--;
            vga_text_buffer[vga_cursor_y * VGA_WIDTH + vga_cursor_x] = (unsigned short)(' ' | (vga_current_color << 8));
        }
    } else {
        unsigned int index = vga_cursor_y * VGA_WIDTH + vga_cursor_x;
        vga_text_buffer[index] = (unsigned short)(c | (vga_current_color << 8));
        vga_cursor_x++;
    }

    if (vga_cursor_x >= VGA_WIDTH) {
        vga_cursor_x = 0;
        vga_cursor_y++;
    }

    if (vga_cursor_y >= VGA_HEIGHT) {
        vga_scroll_up();
        vga_cursor_y = VGA_HEIGHT - 1;
    }

    vga_set_cursor(vga_cursor_x, vga_cursor_y);
}

void vga_print(const char *str) {
    if (!str) {
        return;
    }
    while (*str) {
        vga_putchar(*str);
        str++;
    }
}

void vga_set_color(unsigned char fg, unsigned char bg) {
    vga_current_color = (unsigned char)((bg << 4) | (fg & 0x0F));
}

void vga_write_registers(vga_registers_t *regs) {
    if (!regs) {
        return;
    }

    vga_outb(VGA_MISC_WRITE, regs->misc);

    vga_outb(VGA_SEQ_INDEX, 0x00);
    vga_outb(VGA_SEQ_DATA, 0x01);
    for (unsigned char i = 1; i < 5; i++) {
        vga_outb(VGA_SEQ_INDEX, i);
        vga_outb(VGA_SEQ_DATA, regs->seq[i]);
    }
    vga_outb(VGA_SEQ_INDEX, 0x00);
    vga_outb(VGA_SEQ_DATA, 0x03);

    vga_outb(VGA_CRTC_INDEX, 0x11);
    vga_outb(VGA_CRTC_DATA, regs->crtc[0x11] & 0x7F);
    for (unsigned char i = 0; i < 25; i++) {
        vga_outb(VGA_CRTC_INDEX, i);
        vga_outb(VGA_CRTC_DATA, regs->crtc[i]);
    }

    for (unsigned char i = 0; i < 9; i++) {
        vga_outb(VGA_GC_INDEX, i);
        vga_outb(VGA_GC_DATA, regs->gc[i]);
    }

    for (unsigned char i = 0; i < 21; i++) {
        (void)vga_inb(VGA_AC_READ);
        vga_outb(VGA_AC_INDEX, i);
        vga_outb(VGA_AC_WRITE, regs->ac[i]);
    }
    vga_outb(VGA_AC_INDEX, 0x20);
}

void vga_read_registers(vga_registers_t *regs) {
    if (!regs) {
        return;
    }

    regs->misc = vga_inb(VGA_MISC_READ);

    for (unsigned char i = 0; i < 5; i++) {
        vga_outb(VGA_SEQ_INDEX, i);
        regs->seq[i] = vga_inb(VGA_SEQ_DATA);
    }

    for (unsigned char i = 0; i < 25; i++) {
        vga_outb(VGA_CRTC_INDEX, i);
        regs->crtc[i] = vga_inb(VGA_CRTC_DATA);
    }

    for (unsigned char i = 0; i < 9; i++) {
        vga_outb(VGA_GC_INDEX, i);
        regs->gc[i] = vga_inb(VGA_GC_DATA);
    }

    for (unsigned char i = 0; i < 21; i++) {
        (void)vga_inb(VGA_AC_READ);
        vga_outb(VGA_AC_INDEX, i);
        regs->ac[i] = vga_inb(VGA_AC_READ);
    }
    vga_outb(VGA_AC_INDEX, 0x20);
}