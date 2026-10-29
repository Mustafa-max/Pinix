/*
 * Pinix-Kernel DRM Subsystem - drm.h
 * Direct Rendering Manager Header Compatible with Hardware and GOP Subsystems
 */

#ifndef DRM_H
#define DRM_H

#define DRM_MAX_DEVICES 8
#define KMS_MAX_MODES 16
#define KMS_MAX_FRAMEBUFFERS 4

#define DRM_SUCCESS 0
#define DRM_ERROR_NO_DEVICE -1
#define DRM_ERROR_NOMEM -2
#define DRM_ERROR_INVALID_PARAM -3

typedef struct {
    unsigned int id;
    unsigned long mmio_base;
    unsigned long fb_base;
    unsigned int width;
    unsigned int height;
    unsigned int pitch;
    unsigned int bpp;
    unsigned int initialized;
} drm_device_t;

typedef struct {
    unsigned int clock;
    unsigned int hdisplay;
    unsigned int hsync_start;
    unsigned int hsync_end;
    unsigned int htotal;
    unsigned int vdisplay;
    unsigned int vsync_start;
    unsigned int vsync_end;
    unsigned int vtotal;
} drm_display_mode_t;

typedef struct {
    unsigned int id;
    unsigned int width;
    unsigned int height;
    unsigned int pitch;
    unsigned int bpp;
    void *buffer;
} drm_framebuffer_t;

int drm_init(void);
int drm_register_device(drm_device_t *dev);
int drm_unregister_device(unsigned int id);
drm_device_t* drm_get_primary_device(void);
unsigned int drm_read_reg(drm_device_t *dev, unsigned long offset);
void drm_write_reg(drm_device_t *dev, unsigned long offset, unsigned int val);
int drm_create_client(unsigned int pid);
int drm_authenticate_client(int client_id, unsigned int magic);
int drm_gem_create_object(unsigned long size, unsigned int *handle);
int drm_gem_free_object(unsigned int handle);
int drm_prime_handle_to_fd(unsigned int handle, int *fd);
int drm_prime_fd_to_handle(int fd, unsigned int *handle);
int drm_ioctl(unsigned int request, unsigned long arg);

#endif