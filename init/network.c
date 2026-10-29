/*
 * vmpinix Initialization Subsystem - network.c
 * Terminal Command Line Interface for Network Configuration and Control
 */

#include "../include/stdint.h"

// net/ altındaki ağ fonksiyonlarına referanslar
extern int net_terminal_process_input(const char *ssid, const char *password);
extern int network_disconnect(void);
extern int network_is_connected(void);

int init_network_cli_handler(const char *command, const char *arg1, const char *arg2) {
    if (!command) {
        return -1;
    }

    // 1. "connect <ssid> <password>" Komutu
    int is_connect = 1;
    const char *c_cmd = "connect";
    for (int i = 0; c_cmd[i] != '\0' || command[i] != '\0'; i++) {
        if (command[i] != c_cmd[i]) { is_connect = 0; break; }
    }

    if (is_connect && arg1 && arg2) {
        // Terminalden gelen SSID ve şifreyi net modülüne ilet
        return net_terminal_process_input(arg1, arg2);
    }

    // 2. "disconnect" Komutu
    int is_disc = 1;
    const char *d_cmd = "disconnect";
    for (int i = 0; d_cmd[i] != '\0' || command[i] != '\0'; i++) {
        if (command[i] != d_cmd[i]) { is_disc = 0; break; }
    }

    if (is_disc) {
        return network_disconnect();
    }

    // 3. "status" Komutu
    int is_status = 1;
    const char *s_cmd = "status";
    for (int i = 0; s_cmd[i] != '\0' || command[i] != '\0'; i++) {
        if (command[i] != s_cmd[i]) { is_status = 0; break; }
    }

    if (is_status) {
        return network_is_connected();
    }

    return -2; // Bilinmeyen komut
}