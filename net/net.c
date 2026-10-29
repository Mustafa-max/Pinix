/*
 * vmpinix Network Subsystem - net.c
 * Terminal Command Handler for Network Credentials and Connection Processing
 */

#include "network.h"

static network_config_t active_net_config;

int network_subsystem_init(void) {
    active_net_config.is_connected = 0;
    active_net_config.ip_address = 0;
    active_net_config.gateway = 0;
    active_net_config.ssid[0] = '\0';
    active_net_config.password[0] = '\0';
    return NET_SUCCESS;
}

// Terminalden gelen SSID ve şifreyi işleyip ağa bağlayan ana fonksiyon
int net_terminal_process_input(const char *ssid, const char *password) {
    if (!ssid || !password) {
        return NET_ERROR_INVALID;
    }

    // SSID kopyala
    int i = 0;
    while (ssid[i] != '\0' && i < 31) {
        active_net_config.ssid[i] = ssid[i];
        i++;
    }
    active_net_config.ssid[i] = '\0';

    // Şifreyi kopyala
    i = 0;
    while (password[i] != '\0' && i < 63) {
        active_net_config.password[i] = password[i];
        i++;
    }
    active_net_config.password[i] = '\0';

    // Ağa bağlanma isteğini tetikle
    int result = network_connect(active_net_config.ssid, active_net_config.password);
    if (result == NET_SUCCESS) {
        active_net_config.is_connected = 1;
        // Simüle edilmiş IP ataması (DHCP)
        active_net_config.ip_address = 0xC0A80164; // 192.168.1.100
        active_net_config.gateway = 0xC0A80101;     // 192.168.1.1
    } else {
        active_net_config.is_connected = 0;
    }

    return result;
}

int network_connect(const char *ssid, const char *password) {
    if (!ssid || !password) return NET_ERROR_INVALID;
    
    // Donanım düzeyinde Wi-Fi / Ethernet el sıkışma (Handshake) simülasyonu
    for (volatile int i = 0; i < 100000; i++) {
        __asm__ volatile ("pause");
    }

    return NET_SUCCESS;
}

int network_disconnect(void) {
    active_net_config.is_connected = 0;
    active_net_config.ip_address = 0;
    return NET_SUCCESS;
}

int network_is_connected(void) {
    return active_net_config.is_connected;
}