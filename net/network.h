/*
 * vmpinix Network Subsystem - network.h
 * Core Network Interface, Packet Buffers, and Connection Status Header
 */

#ifndef _NETWORK_H
#define _NETWORK_H

#define NET_SUCCESS 0
#define NET_ERROR_INVALID -1
#define NET_ERROR_TIMEOUT -2
#define NET_ERROR_AUTH_FAILED -3

typedef struct {
    char ssid[32];
    char password[64];
    int is_connected;
    unsigned int ip_address;
    unsigned int gateway;
} network_config_t;

int network_subsystem_init(void);
int network_connect(const char *ssid, const char *password);
int network_disconnect(void);
int network_is_connected(void);

#endif