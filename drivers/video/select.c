#include "video.h"

int gop_probe(void);
int vesa_probe(void);
int drm_probe(void);
int kms_probe(void);

int video_select_backend(void) {
    if (gop_probe()) return VIDEO_BACKEND_GOP;
    if (vesa_probe()) return VIDEO_BACKEND_VESA;
    if (drm_probe()) return VIDEO_BACKEND_DRM;
    if (kms_probe()) return VIDEO_BACKEND_KMS;
    return VIDEO_BACKEND_NONE;
}
