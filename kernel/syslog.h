/*
 * vmpinix Core Kernel - syslog.h
 * System Logging and Kernel Message Buffer Header
 */

#ifndef _KERNEL_SYSLOG_H
#define _KERNEL_SYSLOG_H

#define SYSLOG_EMERG   0
#define SYSLOG_ALERT   1
#define SYSLOG_CRIT    2
#define SYSLOG_ERR     3
#define SYSLOG_WARNING 4
#define SYSLOG_NOTICE  5
#define SYSLOG_INFO    6
#define SYSLOG_DEBUG   7

int syslog_init(void);
void syslog_log(int level, const char *message);
const char* syslog_read_last(void);

#endif