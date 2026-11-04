#include "serial.h"

#define COM1_BASE 0x3F8u
#define UART_DATA (COM1_BASE + 0u)
#define UART_IER  (COM1_BASE + 1u)
#define UART_FCR  (COM1_BASE + 2u)
#define UART_LCR  (COM1_BASE + 3u)
#define UART_MCR  (COM1_BASE + 4u)
#define UART_LSR  (COM1_BASE + 5u)
#define UART_LSR_THRE 0x20u
#define UART_SPIN_LIMIT 1000000u

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

int serial_init(void) {
    outb(UART_IER, 0x00u);
    outb(UART_LCR, 0x80u);
    outb(UART_DATA, 0x01u); /* divisor 1: 115200 baud */
    outb(UART_IER, 0x00u);
    outb(UART_LCR, 0x03u);  /* 8 data bits, no parity, one stop bit */
    outb(UART_FCR, 0xC7u);
    outb(UART_MCR, 0x0Bu);
    return 0;
}

void serial_write_char(char c) {
    for (uint32_t spin = 0; spin < UART_SPIN_LIMIT; ++spin) {
        if ((inb(UART_LSR) & UART_LSR_THRE) != 0) {
            outb(UART_DATA, (uint8_t)c);
            return;
        }
        __asm__ volatile ("pause");
    }
}

void serial_write_string(const char *text) {
    if (text == (const char *)0) {
        return;
    }
    while (*text != '\0') {
        if (*text == '\n') {
            serial_write_char('\r');
        }
        serial_write_char(*text++);
    }
}

void serial_write_hex64(uint64_t value) {
    static const char digits[] = "0123456789ABCDEF";
    serial_write_string("0x");
    for (int shift = 60; shift >= 0; shift -= 4) {
        serial_write_char(digits[(value >> shift) & 0xFu]);
    }
}
