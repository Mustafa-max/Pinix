/*
 * vmpinix System Subsystem - sys.c
 * Dynamic Extension Loader and Automated Plugin Execution Engine
 */

#include "sys.h"

#define MAX_SYSTEM_EXTENSIONS 64

typedef struct {
    sys_extension_t extensions[MAX_SYSTEM_EXTENSIONS];
    int count;
    int initialized;
} sys_manager_t;

static sys_manager_t sys_mgr;

int sys_extensions_init_all(void) {
    if (sys_mgr.initialized) {
        return 0;
    }

    sys_mgr.count = 0;
    for (int i = 0; i < MAX_SYSTEM_EXTENSIONS; i++) {
        sys_mgr.extensions[i].name = (const char*)0;
        sys_mgr.extensions[i].author = (const char*)0;
        sys_mgr.extensions[i].init_func = (sys_extension_init_t)0;
        sys_mgr.extensions[i].active = 0;
    }

    sys_mgr.initialized = 1;
    return 0;
}

int sys_register_extension(const char *name, const char *author, sys_extension_init_t init_func) {
    if (!name || !init_func || sys_mgr.count >= MAX_SYSTEM_EXTENSIONS) {
        return -1;
    }

    sys_extension_t *ext = &sys_mgr.extensions[sys_mgr.count++];
    ext->name = name;
    ext->author = author ? author : "Community Contributor";
    ext->init_func = init_func;
    ext->active = 0;

    // Eklenti kaydedildiği anda başlatmayı dene
    int res = init_func();
    if (res == 0) {
        ext->active = 1;
    }

    return res;
}

unsigned int sys_get_extension_count(void) {
    return (unsigned int)sys_mgr.count;
}