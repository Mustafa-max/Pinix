#ifndef DRIVER_H
#define DRIVER_H

#include <stdint.h>

#define DRIVER_OK 0
#define DRIVER_ERR -1

typedef enum {
    DRIVER_TYPE_GPU,
    DRIVER_TYPE_INPUT,
    DRIVER_TYPE_BLOCK,
    DRIVER_TYPE_UNKNOWN
} driver_type_t;

typedef struct driver driver_t;

typedef int (*driver_init_fn)(void);
typedef int (*driver_probe_fn)(void);

struct driver {
    const char *name;
    driver_type_t type;
    driver_init_fn init;
    driver_probe_fn probe;
    driver_t *next;
};

int driver_register(driver_t *driver);
int driver_unregister(driver_t *driver);
void driver_init_all(void);

#endif