#ifndef PS2_H
#define PS2_H

#define PS2_DATA_PORT 0x60
#define PS2_STATUS_PORT 0x64
#define PS2_COMMAND_PORT 0x64

#define PS2_OK 0
#define PS2_ERR -1

void ps2_wait_read(void);
void ps2_wait_write(void);
unsigned char ps2_read_data(void);
void ps2_write_data(unsigned char value);
void ps2_write_command(unsigned char value);
int ps2_kbd_init(void);
int ps2_mouse_init(void);
unsigned char ps2_kbd_read(void);
unsigned char ps2_mouse_read(void);

#endif