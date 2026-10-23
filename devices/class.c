/*
 * vmpinix Device Subsystem - class.c
 * Device Classification and Functional Grouping Implementation
 */

#include "devices.h"

#define MAX_CLASSES 16

typedef struct {
    unsigned int class_id;
    char name[32];
    unsigned int members[64];
    int member_count;
} device_class_t;

static device_class_t class_table[MAX_CLASSES];
static int class_count = 0;

int class_subsystem_init(void) {
    class_count = 0;
    for (int i = 0; i < MAX_CLASSES; i++) {
        class_table[i].class_id = 0;
        class_table[i].member_count = 0;
        class_table[i].name[0] = '\0';
    }
    return DEVICE_SUCCESS;
}

int class_register(unsigned int class_id, const char *name) {
    if (class_count >= MAX_CLASSES || !name) {
        return DEVICE_ERROR_NOMEM;
    }

    device_class_t *cls = &class_table[class_count++];
    cls->class_id = class_id;
    cls->member_count = 0;

    int i = 0;
    while (name[i] != '\0' && i < 31) {
        cls->name[i] = name[i];
        i++;
    }
    cls->name[i] = '\0';

    return DEVICE_SUCCESS;
}

int class_add_member(unsigned int class_id, unsigned int device_id) {
    for (int i = 0; i < class_count; i++) {
        if (class_table[i].class_id == class_id) {
            if (class_table[i].member_count >= 64) {
                return DEVICE_ERROR_NOMEM;
            }
            class_table[i].members[class_table[i].member_count++] = device_id;
            return DEVICE_SUCCESS;
        }
    }
    return DEVICE_ERROR_NOTFOUND;
}