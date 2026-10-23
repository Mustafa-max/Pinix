/*
 * vmpinix Initialization Subsystem - systemctl.c
 * Comprehensive System Control, Power Management, Service and USB Device Manager
 */

#include "../include/stdint.h"

extern int systemd_start_service(const char *name);
extern int systemd_stop_service(const char *name);
extern int device_register(void *dev);
extern int device_unregister(unsigned int id);

#define CMD_SUCCESS 0
#define CMD_ERROR_INVALID -1
#define CMD_ERROR_UNKNOWN -2

// Donanım güç yönetimi simülasyon portları / fonksiyonları
static void system_power_off(void) {
    // ACPI veya APM üzerinden sistem kapatma sinyali
    __asm__ volatile ("cli; hlt");
    for (;;);
}

static void system_reboot(void) {
    // Klavye kontrolcüsü (8042) üzerinden reset sinyali
    __asm__ volatile (
        "movb $0xFE, %%al\n\t"
        "outb %%al, $0x64\n\t"
        : : : "al"
    );
    for (;;);
}

static int usb_subsystem_control(int enable) {
    // USB Host Controller (xHCI/EHCI) güç ve durum yönetimi
    volatile unsigned int *usb_hc_reg = (volatile unsigned int*)0xFE400000;
    if (enable) {
        *usb_hc_reg |= (1 << 0); // USB Kontrolcüyü Etkinleştir
    } else {
        *usb_hc_reg &= ~(1 << 0); // USB Kontrolcüyü Durdur
    }
    return CMD_SUCCESS;
}

int systemctl_execute(const char *command, const char *target) {
    if (!command) {
        return CMD_ERROR_INVALID;
    }

    // 1. "poweroff" Komutu
    int is_poweroff = 1;
    const char *p_cmd = "poweroff";
    for (int i = 0; p_cmd[i] != '\0' || command[i] != '\0'; i++) {
        if (command[i] != p_cmd[i]) { is_poweroff = 0; break; }
    }
    if (is_poweroff) {
        system_power_off();
        return CMD_SUCCESS;
    }

    // 2. "reboot" Komutu
    int is_reboot = 1;
    const char *r_cmd = "reboot";
    for (int i = 0; r_cmd[i] != '\0' || command[i] != '\0'; i++) {
        if (command[i] != r_cmd[i]) { is_reboot = 0; break; }
    }
    if (is_reboot) {
        system_reboot();
        return CMD_SUCCESS;
    }

    // 3. "start" Servis Başlatma
    int is_start = 1;
    const char *s_cmd = "start";
    for (int i = 0; s_cmd[i] != '\0' || command[i] != '\0'; i++) {
        if (command[i] != s_cmd[i]) { is_start = 0; break; }
    }
    if (is_start && target) {
        return systemd_start_service(target);
    }

    // 4. "stop" Servis Durdurma
    int is_stop = 1;
    const char *st_cmd = "stop";
    for (int i = 0; st_cmd[i] != '\0' || command[i] != '\0'; i++) {
        if (command[i] != st_cmd[i]) { is_stop = 0; break; }
    }
    if (is_stop && target) {
        return systemd_stop_service(target);
    }

    // 5. "usb" Yönetimi (enable / disable)
    int is_usb = 1;
    const char *u_cmd = "usb";
    for (int i = 0; u_cmd[i] != '\0' || command[i] != '\0'; i++) {
        if (command[i] != u_cmd[i]) { is_usb = 0; break; }
    }
    if (is_usb && target) {
        int enable = (target[0] == 'o' && target[1] == 'n') ? 1 : 0;
        return usb_subsystem_control(enable);
    }

    return CMD_ERROR_UNKNOWN;
}