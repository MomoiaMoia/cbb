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

/* ------------------------------------------------------------------ */
/*  Unsharp-mask sharpening / deblurring                              */
/*                                                                     */
/*  dst = src + amount * (src - blurred)                              */
/*  A 3×3 Gaussian blur (σ≈0.8) is used for the blurred layer.        */
/*  amount=0.5 ~ 1.5  (0 = no effect, >1 = strong sharpening)         */
/*                                                                     */
/*  work_buffer must be ≥ width * height * 3 bytes (scratch).         */
/*  src and dst may point to the same buffer (in-place OK).           */
/* ------------------------------------------------------------------ */
void unsharp_mask_rgb888(const uint8_t *src, uint8_t *dst,
                         int width, int height, float amount,
                         uint8_t *work_buffer);

/* ------------------------------------------------------------------ */
/*  Geometric distortion correction (径向畸变矫正)                     */
/*                                                                     */
/*  Corrects barrel / pincushion distortion using the radial model:    */
/*    x_src = cx + (x_dst - cx) * (1 + k1 * r² + k2 * r⁴)            */
/*    y_src = cy + (y_dst - cy) * (1 + k1 * r² + k2 * r⁴)            */
/*                                                                     */
/*  For barrel  (中间鼓) → k1 < 0  (e.g. -0.1)                        */
/*  For pincushion (四角拉伸) → k1 > 0                                 */
/*                                                                     */
/*  aspect: 宽高比修正因子 (>1 = 横向拉伸, <1 = 纵向拉伸)             */
/*          如果画面中心圆形目标呈高度>宽度椭圆, 尝试 aspect=1.03      */
/*                                                                     */
/*  src == dst 支持原地操作 (仅当 k1 <= 0 时安全)。                    */
/* ------------------------------------------------------------------ */
void geo_correct_rgb888(const uint8_t *src, uint8_t *dst,
                        int width, int height,
                        float k1, float k2, float aspect);

#endif
