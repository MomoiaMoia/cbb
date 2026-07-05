#ifndef _IMAGE_UTILS_H_
#define _IMAGE_UTILS_H_

#include <stdint.h>
#include "zf_device_scc8660.h"

#define RGB_BUF_SIZE    (SCC8660_W * SCC8660_H * 3)

typedef struct {
    uint16_t sync;
    uint32_t size;
} jpeg_header_t;

/* JPEG compress YUYV → JPEG */
int  jpeg_compress_yuyv(uint8_t *pInput, int width, int height,
                        uint8_t *pOutput, int outputSize);

/* YUYV → RGB888
   flip=0: 正常顺序, flip=1: 垂直翻转 (摄像头倒装时使用) */
void yuyv_to_rgb888(const uint8_t *pInput, int width, int height,
                    uint8_t *pOutput, int flip);

#endif
