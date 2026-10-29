#include "../include/driver.h"

static driver_t *g_driver_list = 0;

int driver_register(driver_t *driver) {
    if (!driver) return DRIVER_ERR;
    driver->next = g_driver_list;
    g_driver_list = driver;
    return DRIVER_OK;
}

int driver_unregister(driver_t *driver) {
    driver_t *prev = 0;
    driver_t *cur = g_driver_list;

    while (cur) {
        if (cur == driver) {
            if (prev) prev->next = cur->next;
            else g_driver_list = cur->next;
            return DRIVER_OK;
        }
        prev = cur;
        cur = cur->next;
    }

    return DRIVER_ERR;
}

void driver_init_all(void) {
    driver_t *d = g_driver_list;

    while (d) {
        if (d->probe && d->probe() == DRIVER_OK) {
            if (d->init) {
                d->init();
            }
        }
        d = d->next;
    }
}