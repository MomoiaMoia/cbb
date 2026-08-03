// image_utils.h — Image processing utilities (JPEG, YUYV→RGB888, sharpening, geometry)
#ifndef _IMAGE_UTILS_H_
#define _IMAGE_UTILS_H_

#include "zf_device_scc8660.h"
#include <stdint.h>

#define RGB_BUF_SIZE (SCC8660_W * SCC8660_H * 3)

typedef struct {
    uint16_t sync;
    uint32_t size;
} jpeg_header_t;

// JPEG compress YUYV → JPEG
int jpeg_compress_yuyv(uint8_t *pInput, int width, int height,
                       uint8_t *pOutput, int outputSize);

// YUYV → RGB888 conversion; flip=0: normal, flip=1: vertical flip
void yuyv_to_rgb888(const uint8_t *pInput, int width, int height,
                    uint8_t *pOutput, int flip);

// Unsharp-mask sharpening: dst = src + amount*(src - gaussian_blur(src))
// 3×3 Gaussian kernel, amount=0.5~1.5, work_buffer ≥ width*height*3 bytes
void unsharp_mask_rgb888(const uint8_t *src, uint8_t *dst,
                         int width, int height, float amount,
                         uint8_t *work_buffer);

// Geometric distortion correction (radial model): k1<0=barrel, k1>0=pincushion
// aspect: aspect ratio correction (>1=horiz stretch, <1=vert stretch)
void geo_correct_rgb888(const uint8_t *src, uint8_t *dst,
                        int width, int height,
                        float k1, float k2, float aspect);

#endif
