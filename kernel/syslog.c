/*
 * vmpinix Core Kernel - syslog.c
 * Kernel Logging Engine, Circular Buffer and Severity Handler
 */

#include "syslog.h"

#define SYSLOG_BUFFER_SIZE 1024
#define MAX_LOG_MSG_LEN 128

typedef struct {
    char messages[SYSLOG_BUFFER_SIZE][MAX_LOG_MSG_LEN];
    int levels[SYSLOG_BUFFER_SIZE];
    int head;
    int count;
} syslog_registry_t;

static syslog_registry_t log_reg;

int syslog_init(void) {
    log_reg.head = 0;
    log_reg.count = 0;
    for (int i = 0; i < SYSLOG_BUFFER_SIZE; i++) {
        log_reg.levels[i] = 0;
        log_reg.messages[i][0] = '\0';
    }
    return 0;
}

void syslog_log(int level, const char *message) {
    if (!message) return;

    int idx = log_reg.head;
    log_reg.levels[idx] = level;

    // Mesajı güvenli kopyala
    int i = 0;
    while (message[i] != '\0' && i < MAX_LOG_MSG_LEN - 1) {
        log_reg.messages[idx][i] = message[i];
        i++;
    }
    log_reg.messages[idx][i] = '\0';

    // Dairesel tampon indeksini ilerlet
    log_reg.head = (log_reg.head + 1) % SYSLOG_BUFFER_SIZE;
    if (log_reg.count < SYSLOG_BUFFER_SIZE) {
        log_reg.count++;
    }
}

const char* syslog_read_last(void) {
    if (log_reg.count == 0) {
        return (const char*)0;
    }
    
    int last_idx = (log_reg.head - 1 + SYSLOG_BUFFER_SIZE) % SYSLOG_BUFFER_SIZE;
    return log_reg.messages[last_idx];
}