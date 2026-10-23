/*
 * vmpinix Initialization Subsystem - systemd.c
 * Service Manager, Daemon Registry, and Lifecycle Controller
 */

#include "../include/stdint.h"
#include "../include/string.h"

#define MAX_SERVICES 32
#define SERVICE_NAME_LEN 32

typedef enum {
    SERVICE_STATE_STOPPED = 0,
    SERVICE_STATE_RUNNING = 1,
    SERVICE_STATE_FAILED = 2
} service_state_t;

typedef struct {
    char name[SERVICE_NAME_LEN];
    service_state_t state;
    int pid;
    int (*start_func)(void);
    int (*stop_func)(void);
} system_service_t;

static system_service_t service_registry[MAX_SERVICES];
static int service_count = 0;

int systemd_init(void) {
    service_count = 0;
    for (int i = 0; i < MAX_SERVICES; i++) {
        service_registry[i].name[0] = '\0';
        service_registry[i].state = SERVICE_STATE_STOPPED;
        service_registry[i].pid = -1;
        service_registry[i].start_func = (void*)0;
        service_registry[i].stop_func = (void*)0;
    }
    return 0;
}

int systemd_register_service(const char *name, int (*start)(void), int (*stop)(void)) {
    if (!name || service_count >= MAX_SERVICES) {
        return -1;
    }

    system_service_t *svc = &service_registry[service_count++];
    int i = 0;
    while (name[i] != '\0' && i < SERVICE_NAME_LEN - 1) {
        svc->name[i] = name[i];
        i++;
    }
    svc->name[i] = '\0';
    svc->state = SERVICE_STATE_STOPPED;
    svc->start_func = start;
    svc->stop_func = stop;

    return 0;
}

int systemd_start_service(const char *name) {
    for (int i = 0; i < service_count; i++) {
        int match = 1;
        int j = 0;
        while (service_registry[i].name[j] != '\0' && name[j] != '\0') {
            if (service_registry[i].name[j] != name[j]) {
                match = 0;
                break;
            }
            j++;
        }
        if (match && service_registry[i].name[j] == name[j]) {
            if (service_registry[i].state == SERVICE_STATE_RUNNING) {
                return 0; // Zaten çalışıyor
            }
            if (service_registry[i].start_func) {
                int res = service_registry[i].start_func();
                if (res == 0) {
                    service_registry[i].state = SERVICE_STATE_RUNNING;
                    return 0;
                }
            }
            service_registry[i].state = SERVICE_STATE_FAILED;
            return -1;
        }
    }
    return -2; // Bulunamadı
}

int systemd_stop_service(const char *name) {
    for (int i = 0; i < service_count; i++) {
        int match = 1;
        int j = 0;
        while (service_registry[i].name[j] != '\0' && name[j] != '\0') {
            if (service_registry[i].name[j] != name[j]) {
                match = 0;
                break;
            }
            j++;
        }
        if (match && service_registry[i].name[j] == name[j]) {
            if (service_registry[i].state == SERVICE_STATE_STOPPED) {
                return 0;
            }
            if (service_registry[i].stop_func) {
                service_registry[i].stop_func();
            }
            service_registry[i].state = SERVICE_STATE_STOPPED;
            return 0;
        }
    }
    return -1;
}