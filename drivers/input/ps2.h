#ifndef PS2_H
#define PS2_H

#include "../../include/stdint.h"

#define PS2_OK 0
#define PS2_ERR -1

int ps2_kbd_init(void);
int ps2_mouse_init(void);
int ps2_kbd_try_read(uint8_t *scancode);
int ps2_mouse_try_read(uint8_t *data);

/* Compatibility helpers: return 0 on timeout/error. Prefer try_read APIs. */
unsigned char ps2_kbd_read(void);
unsigned char ps2_mouse_read(void);

#endif
