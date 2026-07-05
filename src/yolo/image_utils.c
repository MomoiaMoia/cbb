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
