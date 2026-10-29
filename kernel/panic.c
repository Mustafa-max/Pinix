/*
 * vmpinix Core Kernel - panic.c
 * Direct VGA Text Mode Kernel Panic Screen and Error Display Implementation
 */

#define VGA_TEXT_BUFFER ((volatile unsigned short*)0xB8000)
#define VGA_COLOR_PANIC 0x4F // Kırmızı Arka Plan, Parlak Beyaz Yazı

__attribute__((noreturn)) void kernel_panic(int error_code, const char *reason) {
    // 1. Tüm donanım kesmelerini (interrupts) tamamen durdur
    __asm__ volatile ("cli");

    // 2. VGA Metin Modu Belleğine Doğrudan Erişim (80x25 Ekran)
    volatile unsigned short *vga = VGA_TEXT_BUFFER;

    // Ekranı kırmızı panik arka planı ile tamamen temizle
    for (int i = 0; i < 80 * 25; i++) {
        vga[i] = (VGA_COLOR_PANIC << 8) | ' ';
    }

    // 3. Başlık Yazdır: "=== vmpinix KERNEL PANIC ==="
    const char *title = "=== vmpinix KERNEL PANIC ===";
    int row = 2, col = 4;
    for (int i = 0; title[i] != '\0'; i++) {
        vga[row * 80 + col + i] = (VGA_COLOR_PANIC << 8) | title[i];
    }

    // 4. Hata Kodunu Yazdır
    const char *err_text = "ERROR CODE: ";
    row = 5; col = 4;
    for (int i = 0; err_text[i] != '\0'; i++) {
        vga[row * 80 + col + i] = (VGA_COLOR_PANIC << 8) | err_text[i];
    }

    // Hata kodunu sayısal olarak ekrana çevir ve bas
    int temp_code = error_code;
    char code_str[12];
    int idx = 0;
    if (temp_code == 0) {
        code_str[idx++] = '0';
    } else {
        while (temp_code > 0) {
            code_str[idx++] = '0' + (temp_code % 10);
            temp_code /= 10;
        }
    }
    for (int i = 0; i < idx; i++) {
        char digit = code_str[idx - 1 - i];
        vga[row * 80 + col + 12 + i] = (VGA_COLOR_PANIC << 8) | digit;
    }

    // 5. Hata Sebebi / Açıklamasını Yazdır
    const char *reason_label = "REASON: ";
    row = 7; col = 4;
    for (int i = 0; reason_label[i] != '\0'; i++) {
        vga[row * 80 + col + i] = (VGA_COLOR_PANIC << 8) | reason_label[i];
    }

    if (reason) {
        for (int i = 0; reason[i] != '\0' && i < 60; i++) {
            vga[row * 80 + col + 8 + i] = (VGA_COLOR_PANIC << 8) | reason[i];
        }
    }

    // 6. Sistem Durum Alt Bilgisi
    const char *footer = "System halted safely. Please reboot your computer.";
    row = 10; col = 4;
    for (int i = 0; footer[i] != '\0'; i++) {
        vga[row * 80 + col + i] = (VGA_COLOR_PANIC << 8) | footer[i];
    }

    // 7. Donanımı tamamen askıya al ve sonsuz döngüye sok
    for (;;) {
        __asm__ volatile ("hlt");
    }
}