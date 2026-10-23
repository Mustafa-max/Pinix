#include "../../include/driver.h"

static int ps2_probe(void) {
    return DRIVER_OK;
}

static int ps2_init(void) {
    return DRIVER_OK;
}

static driver_t ps2_driver = {
    .name = "ps2",
    .type = DRIVER_TYPE_INPUT,
    .probe = ps2_probe,
    .init = ps2_init,
    .next = 0
};

int ps2_driver_register(void) {
    return driver_register(&ps2_driver);
}