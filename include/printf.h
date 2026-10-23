/*
 * vmpinix Standard Library - printf.h
 * Formatted Output Engine Header Definitions
 */

#ifndef _PRINTF_H
#define _PRINTF_H

#include <stdarg.h>

int printf(const char *format, ...);
int sprintf(char *str, const char *format, ...);
int vsprintf(char *str, const char *format, va_list ap);

#endif