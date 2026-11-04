#ifndef PINIX_SERIAL_H
#define PINIX_SERIAL_H

#include "../../include/stdint.h"

int serial_init(void);
void serial_write_char(char c);
void serial_write_string(const char *text);
void serial_write_hex64(uint64_t value);

#endif
