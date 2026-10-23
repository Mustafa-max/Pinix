/*
 * vmpinix System Subsystem - sys.h
 * Open-Source Extension and Plugin Registration Framework Header
 */

#ifndef _SYS_H
#define _SYS_H

typedef int (*sys_extension_init_t)(void);

typedef struct {
    const char *name;
    const char *author;
    sys_extension_init_t init_func;
    int active;
} sys_extension_t;

int sys_register_extension(const char *name, const char *author, sys_extension_init_t init_func);
int sys_extensions_init_all(void);
unsigned int sys_get_extension_count(void);

#endif