#include "../include/driver.h"

static int ide_probe(void) {
    return DRIVER_OK;
}

static int ide_init(void) {
    return DRIVER_OK;
}

static driver_t ide_driver = {
    .name = "ide",
    .type = DRIVER_TYPE_BLOCK,
    .probe = ide_probe,
    .init = ide_init,
    .next = 0
};

int ide_driver_register(void) {
    return driver_register(&ide_driver);
}