/*
 * vmpinix Standard Library - stdio.h
 * Standard Input/Output and Console Operations Header
 */

#ifndef _STDIO_H
#define _STDIO_H

#include "printf.h"
#include "stdint.h"

#define EOF (-1)

typedef struct {
    int fd;
    int flags;
} FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

int putchar(int c);
int puts(const char *s);
int getchar(void);

#endif