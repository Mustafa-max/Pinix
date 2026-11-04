# Pinix early graphics pipeline

`pixel.h` defines RGB color values and BGRX pixel conversion. `drm.c` is currently a
small software renderer (not a native GPU driver or Linux-compatible DRM subsystem):
it validates and draws into the framebuffer supplied by the Multiboot2 loader.
`kms.c` validates the currently selected display mode and provides the display helper
layer. It intentionally refuses unsupported mode changes rather than pretending to
program GPU registers.

For UEFI the external loader can obtain a framebuffer from GOP; for legacy BIOS it
can obtain one through VBE before entering the kernel. After handoff, the kernel uses
the common Multiboot2 framebuffer tag and does not call GOP or BIOS interrupts.
The 1920x1080x32 mode is a request only; firmware, loader, or adapter may select a
different mode. The desktop drawing currently paints a blue background and black
bottom panel. It is not yet a full text/UI/window manager, input-driven shell, mouse
cursor, hardware accelerated GPU driver, or refresh-rate-controlled display stack.
