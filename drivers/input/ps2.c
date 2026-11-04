#include "ps2.h"

#define PS2_DATA_PORT 0x60u
#define PS2_STATUS_PORT 0x64u
#define PS2_COMMAND_PORT 0x64u
#define PS2_STATUS_OUTPUT_FULL 0x01u
#define PS2_STATUS_INPUT_FULL 0x02u
#define PS2_STATUS_AUX_DATA 0x20u
#define PS2_WAIT_LIMIT 100000u

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static int wait_input_clear(void) {
    for (uint32_t i = 0; i < PS2_WAIT_LIMIT; ++i) {
        if ((inb(PS2_STATUS_PORT) & PS2_STATUS_INPUT_FULL) == 0) return 0;
        __asm__ volatile ("pause");
    }
    return -1;
}

static int wait_output_full(void) {
    for (uint32_t i = 0; i < PS2_WAIT_LIMIT; ++i) {
        if ((inb(PS2_STATUS_PORT) & PS2_STATUS_OUTPUT_FULL) != 0) return 0;
        __asm__ volatile ("pause");
    }
    return -1;
}

static int controller_command(uint8_t command) {
    if (wait_input_clear() != 0) return -1;
    outb(PS2_COMMAND_PORT, command);
    return 0;
}

static int keyboard_command(uint8_t command) {
    if (wait_input_clear() != 0) return -1;
    outb(PS2_DATA_PORT, command);
    if (wait_output_full() != 0) return -1;
    return inb(PS2_DATA_PORT) == 0xFAu ? 0 : -1;
}

static int mouse_command(uint8_t command) {
    if (controller_command(0xD4u) != 0 || wait_input_clear() != 0) return -1;
    outb(PS2_DATA_PORT, command);
    if (wait_output_full() != 0) return -1;
    return inb(PS2_DATA_PORT) == 0xFAu ? 0 : -1;
}

int ps2_kbd_init(void) {
    if (controller_command(0xADu) != 0) return PS2_ERR;
    if (controller_command(0xAEu) != 0) return PS2_ERR;

    /* Disable scanning then enable it, checking both device acknowledgements. */
    if (keyboard_command(0xF5u) != 0) return PS2_ERR;
    if (keyboard_command(0xF4u) != 0) return PS2_ERR;
    return PS2_OK;
}

int ps2_mouse_init(void) {
    if (controller_command(0xA8u) != 0) return PS2_ERR;
    if (mouse_command(0xF5u) != 0) return PS2_ERR;
    if (mouse_command(0xF4u) != 0) return PS2_ERR;
    return PS2_OK;
}

static int try_read_device(uint8_t *data, int want_aux) {
    if (data == (uint8_t *)0) return PS2_ERR;
    uint8_t status = inb(PS2_STATUS_PORT);
    if ((status & PS2_STATUS_OUTPUT_FULL) == 0) return 0;
    if (((status & PS2_STATUS_AUX_DATA) != 0) != (want_aux != 0)) return 0;
    *data = inb(PS2_DATA_PORT);
    return 1;
}

int ps2_kbd_try_read(uint8_t *scancode) {
    return try_read_device(scancode, 0);
}

int ps2_mouse_try_read(uint8_t *data) {
    return try_read_device(data, 1);
}

unsigned char ps2_kbd_read(void) {
    uint8_t value = 0;
    (void)ps2_kbd_try_read(&value);
    return value;
}

unsigned char ps2_mouse_read(void) {
    uint8_t value = 0;
    (void)ps2_mouse_try_read(&value);
    return value;
}
