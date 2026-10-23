#include "ps2.h"

static inline unsigned char inb(unsigned short port) {
    unsigned char v;
    __asm__ volatile ("inb %1, %0" : "=a"(v) : "Nd"(port));
    return v;
}

static inline void outb(unsigned short port, unsigned char value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

void ps2_wait_read(void) {
    while ((inb(PS2_STATUS_PORT) & 0x01) == 0) {}
}

void ps2_wait_write(void) {
    while ((inb(PS2_STATUS_PORT) & 0x02) != 0) {}
}

unsigned char ps2_read_data(void) {
    ps2_wait_read();
    return inb(PS2_DATA_PORT);
}

void ps2_write_data(unsigned char value) {
    ps2_wait_write();
    outb(PS2_DATA_PORT, value);
}

void ps2_write_command(unsigned char value) {
    ps2_wait_write();
    outb(PS2_COMMAND_PORT, value);
}

int ps2_kbd_init(void) {
    ps2_write_command(0xAD); // disable first port
    ps2_write_command(0xA7); // disable second port
    ps2_write_command(0xAE); // enable first port
    return PS2_OK;
}

int ps2_mouse_init(void) {
    ps2_write_command(0xA8); // enable second port
    return PS2_OK;
}

unsigned char ps2_kbd_read(void) {
    return ps2_read_data();
}

unsigned char ps2_mouse_read(void) {
    return ps2_read_data();
}