#include "image_utils.h"
#include "jpeg/JPEGENC.h"

/* Pull in the actual JPEG encoder core (pure C) */
#include "jpeg/jpegenc.inl"

/* ------------------------------------------------------------------ */
/*  JPEG compress YUYV → JPEG                                         */
/*  YUYV = [Y0, U, Y1, V]  (2 pixels per 4-byte group)               */
/* ------------------------------------------------------------------ */
int jpeg_compress_yuyv(uint8_t *pInput, int width, int height,
                       uint8_t *pOutput, int outputSize) {
    JPEGE_IMAGE jpeg;
    JPEGENCODE enc;
    int rc;

    memset(&jpeg, 0, sizeof(jpeg));
    jpeg.pOutput = pOutput;
    jpeg.iBufferSize = outputSize;
    jpeg.pHighWater = &pOutput[outputSize - 512];

    rc = JPEGEncodeBegin(&jpeg, &enc,
                         width, height,
                         JPEGE_PIXEL_YUV422,
                         JPEGE_SUBSAMPLE_420,
                         JPEGE_Q_MED);
    if (rc != JPEGE_SUCCESS)
        return rc;

    rc = JPEGAddFrame(&jpeg, &enc, pInput, width * 2);
    if (rc != JPEGE_SUCCESS)
        return rc;

    return JPEGEncodeEnd(&jpeg);
}

/* ------------------------------------------------------------------ */
/*  YUYV (Y0,U,Y1,V)  →  RGB888                                       */
/*                                                                     */
/*  YCbCr → RGB (ITU-R BT.601, fixed-point Q10):                      */
/*    R = Y + 1.402   * (Cr - 128)                                     */
/*    G = Y - 0.34414 * (Cb - 128) - 0.71414 * (Cr - 128)             */
/*    B = Y + 1.772   * (Cb - 128)                                     */
/*                                                                     */
/*  Input YUYV:  [Y0, U, Y1, V]                                       */
/*  Output RGB:  [R0,G0,B0, R1,G1,B1]                                 */
/* ------------------------------------------------------------------ */
void yuyv_to_rgb888(const uint8_t *pInput, int width, int height,
                    uint8_t *pOutput, int flip) {
    int row_stride = width * 2;   /* YUYV: 2 bytes/pixel */
    int out_stride = width * 3;   /* RGB888: 3 bytes/pixel */

    for (int row = 0; row < height; row++) {
        /* 垂直翻转: flip=1 时从最后一行往前读 */
        int src_row = flip ? (height - 1 - row) : row;
        const uint8_t *pSrc  = pInput  + src_row * row_stride;
        uint8_t       *pDst  = pOutput + row * out_stride;

        for (int col = 0; col < width; col += 2) {
            int y0 = (int)pSrc[0];           /* Y0     */
            int u  = (int)pSrc[1] - 128;     /* Cb (U) */
            int y1 = (int)pSrc[2];           /* Y1     */
            int v  = (int)pSrc[3] - 128;     /* Cr (V) */

            /* Pixel 0: Y0 + U/V */
            int r0 = y0 + ((1436 * v + 512) >> 10);
            int g0 = y0 - ((352 * u + 731 * v + 512) >> 10);
            int b0 = y0 + ((1815 * u + 512) >> 10);

            /* Pixel 1: Y1 + U/V (same U, V) */
            int r1 = y1 + ((1436 * v + 512) >> 10);
            int g1 = y1 - ((352 * u + 731 * v + 512) >> 10);
            int b1 = y1 + ((1815 * u + 512) >> 10);

            /* Clamp & store */
            pDst[0] = (uint8_t)(r0 < 0 ? 0 : (r0 > 255 ? 255 : r0));
            pDst[1] = (uint8_t)(g0 < 0 ? 0 : (g0 > 255 ? 255 : g0));
            pDst[2] = (uint8_t)(b0 < 0 ? 0 : (b0 > 255 ? 255 : b0));
            pDst[3] = (uint8_t)(r1 < 0 ? 0 : (r1 > 255 ? 255 : r1));
            pDst[4] = (uint8_t)(g1 < 0 ? 0 : (g1 > 255 ? 255 : g1));
            pDst[5] = (uint8_t)(b1 < 0 ? 0 : (b1 > 255 ? 255 : b1));

            pSrc += 4;
            pDst += 6;
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Geometric distortion correction (径向畸变矫正)                     */
/* ------------------------------------------------------------------ */
void geo_correct_rgb888(const uint8_t *src, uint8_t *dst,
                        int width, int height,
                        float k1, float k2, float aspect)
{
    /* Optical centre = image centre */
    const float cx = (width  - 1) * 0.5f;
    const float cy = (height - 1) * 0.5f;
    const float max_r2 = cx * cx + cy * cy;
    if (max_r2 < 1.0f) return;

    const int stride = width * 3;   /* RGB888 bytes per row */

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            /* Normalised offset from optical centre */
            float dx = (float)x - cx;
            float dy = (float)y - cy;

            /* Apply aspect-ratio pre-correction */
            dx /= aspect;

            float r2 = (dx * dx + dy * dy) / max_r2;
            float scale = 1.0f + k1 * r2 + k2 * (r2 * r2);

            float src_x = cx + dx * scale;
            float src_y = cy + dy * scale;

            /* ---- Bilinear interpolation ---- */
            int x0 = (int)src_x;
            int y0 = (int)src_y;
            int x1 = x0 + 1;
            int y1 = y0 + 1;

            /* Clamp to image bounds */
            if (x0 < 0)   x0 = 0;
            if (x0 >= width)  x0 = width - 1;
            if (x1 < 0)   x1 = 0;
            if (x1 >= width)  x1 = width - 1;
            if (y0 < 0)   y0 = 0;
            if (y0 >= height) y0 = height - 1;
            if (y1 < 0)   y1 = 0;
            if (y1 >= height) y1 = height - 1;

            const uint8_t *p00 = src + y0 * stride + x0 * 3;
            const uint8_t *p10 = src + y0 * stride + x1 * 3;
            const uint8_t *p01 = src + y1 * stride + x0 * 3;
            const uint8_t *p11 = src + y1 * stride + x1 * 3;

            const float fx = src_x - (float)x0;
            const float fy = src_y - (float)y0;

            uint8_t *out = dst + y * stride + x * 3;

            for (int c = 0; c < 3; c++) {
                float v = (1.0f - fx) * (1.0f - fy) * (float)p00[c]
                        +        fx  * (1.0f - fy) * (float)p10[c]
                        + (1.0f - fx) *        fy   * (float)p01[c]
                        +        fx  *        fy   * (float)p11[c];
                out[c] = (uint8_t)(v + 0.5f);
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Unsharp-mask sharpening                                            */
/* ------------------------------------------------------------------ */
void unsharp_mask_rgb888(const uint8_t *src, uint8_t *dst,
                         int width, int height, float amount,
                         uint8_t *work_buffer)
{
    /*
     * 3×3 Gaussian kernel (σ≈0.8):
     *   [1  2  1]
     *   [2  4  2]  / 16
     *   [1  2  1]
     *
     * Step 1 — convolve src → work_buffer
     */
    const int stride = width * 3;
    const int last_col = width - 1;
    const int last_row = height - 1;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            uint8_t *wrk = work_buffer + y * stride + x * 3;
            const uint8_t *p;

            /* ---- Row y-1 ---- */
            int sy = (y > 0) ? y - 1 : 0;
            p = src + sy * stride + x * 3;
            if (x > 0) {
                wrk[0] = (p[-3] * 1);
                wrk[1] = (p[-2] * 1);
                wrk[2] = (p[-1] * 1);
            } else {
                wrk[0] = (p[0] * 1);
                wrk[1] = (p[1] * 1);
                wrk[2] = (p[2] * 1);
            }
            wrk[0] += (p[0] * 2);
            wrk[1] += (p[1] * 2);
            wrk[2] += (p[2] * 2);
            if (x < last_col) {
                wrk[0] += (p[3] * 1);
                wrk[1] += (p[4] * 1);
                wrk[2] += (p[5] * 1);
            } else {
                wrk[0] += (p[0] * 1);
                wrk[1] += (p[1] * 1);
                wrk[2] += (p[2] * 1);
            }

            /* ---- Row y ---- */
            p = src + y * stride + x * 3;
            if (x > 0) {
                wrk[0] += (p[-3] * 2);
                wrk[1] += (p[-2] * 2);
                wrk[2] += (p[-1] * 2);
            } else {
                wrk[0] += (p[0] * 2);
                wrk[1] += (p[1] * 2);
                wrk[2] += (p[2] * 2);
            }
            wrk[0] += (p[0] * 4);
            wrk[1] += (p[1] * 4);
            wrk[2] += (p[2] * 4);
            if (x < last_col) {
                wrk[0] += (p[3] * 2);
                wrk[1] += (p[4] * 2);
                wrk[2] += (p[5] * 2);
            } else {
                wrk[0] += (p[0] * 2);
                wrk[1] += (p[1] * 2);
                wrk[2] += (p[2] * 2);
            }

            /* ---- Row y+1 ---- */
            sy = (y < last_row) ? y + 1 : last_row;
            p = src + sy * stride + x * 3;
            if (x > 0) {
                wrk[0] += (p[-3] * 1);
                wrk[1] += (p[-2] * 1);
                wrk[2] += (p[-1] * 1);
            } else {
                wrk[0] += (p[0] * 1);
                wrk[1] += (p[1] * 1);
                wrk[2] += (p[2] * 1);
            }
            wrk[0] += (p[0] * 2);
            wrk[1] += (p[1] * 2);
            wrk[2] += (p[2] * 2);
            if (x < last_col) {
                wrk[0] += (p[3] * 1);
                wrk[1] += (p[4] * 1);
                wrk[2] += (p[5] * 1);
            } else {
                wrk[0] += (p[0] * 1);
                wrk[1] += (p[1] * 1);
                wrk[2] += (p[2] * 1);
            }

            /* Divide by 16 */
            wrk[0] >>= 4;
            wrk[1] >>= 4;
            wrk[2] >>= 4;
        }
    }

    /*
     * Step 2 — dst = src + amount * (src - blurred)
     *          amount is converted to Q8.8 fixed-point for speed
     */
    int amount_q8 = (int)(amount * 256.0f);
    int total = width * height * 3;

    for (int i = 0; i < total; i++) {
        int diff = (int)src[i] - (int)work_buffer[i];
        int val  = (int)src[i] + ((amount_q8 * diff) >> 8);
        if (val < 0)   val = 0;
        if (val > 255) val = 255;
        dst[i] = (uint8_t)val;
    }
}
