#include "hal_data.h"

#include "yolo/image_utils.h"
#include "yolo/yolo_api.h"

#include "common_utils.h"
#include "zf_common_headfile.h"
#include "zf_device_ips200.h"

#include "servos/servo0.h"
#include "servos/servo1.h"
#include "servos/servo2.h"
#include "servos/servo_catch.h"

#include "control/state_machine.h"

#include "zf_device_dl1b.h"

#include <stdbool.h>
#include <stdint.h>

#if (1 == BSP_MULTICORE_PROJECT) && BSP_TZ_SECURE_BUILD
bsp_ipc_semaphore_handle_t g_core_start_semaphore =
{
    .semaphore_num = 0
};
#endif

FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event);
FSP_CPP_FOOTER

/* Image processing buffers */
static uint8_t g_rgb_buffer[RGB_BUF_SIZE] BSP_ALIGN_VARIABLE(8);
/* Corrected RGB888 buffer (geometric distortion correction output) */
static uint8_t g_rgb_corrected[RGB_BUF_SIZE] BSP_ALIGN_VARIABLE(8);

/* Geometric distortion parameters (径向畸变) */
#define GEO_K1     -0.12f    /* barrel distortion (中间鼓) */
#define GEO_K2      0.00f    /* higher-order term */
#define GEO_ASPECT  1.03f    /* aspect ratio correction (中心椭圆补偿) */

/* RGB565 display buffer for IPS200 (160x160 image in RGB565 format) */
static uint16_t g_rgb565_buffer[SCC8660_W * SCC8660_H] BSP_ALIGN_VARIABLE(8);

/* State machine instance */
static StateMachine g_state_machine;

/* ---- debug_init() from zf_common_debug provides _write() redirect ---- */

/* ---- RGB888 -> RGB565 conversion for IPS200 display ---- */
static void rgb888_to_rgb565_buffer(const uint8_t *src, uint16_t *dst, int pixels)
{
    for (int i = 0; i < pixels; i++) {
        uint8_t r = src[i * 3];
        uint8_t g = src[i * 3 + 1];
        uint8_t b = src[i * 3 + 2];
        dst[i] = (uint16_t)(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
    }
}

void hal_entry(void)
{
    fsp_err_t err = FSP_SUCCESS;

    /* ---- Init debug (UART9 + printf redirect from zf_common) ---- */
    debug_init();
    printf("\r\n[INFO] hal_entry() started\r\n");

    /* Open IOPORT module first — needed by IPS200 and other GPIO operations */
    err = g_ioport.p_api->open(g_ioport.p_ctrl, g_ioport.p_cfg);
    handle_error(err, " ** IOPORT OPEN FAILED");

    /* Initialize NPU and YOLO */
    err = g_rm_ethosu0.p_api->open(g_rm_ethosu0.p_ctrl, g_rm_ethosu0.p_cfg);
    handle_error(err, " ** NPU OPEN FAILED");
    static YoloApi yolo_api;
    YoloApi_Init(&yolo_api);

    /* Initialize state machine */
    StateMachine_Init(&g_state_machine);
    printf("[SM] StateMachine initialised\r\n");

    /* Initialize peripherals: UART, SDRAM, SCC8660 camera */
    uint32_t frame_count = 0U;

    R_BSP_SdramInit(true);

    /* ---- Init SCC8660 camera (soft IIC + CEU) ---- */
    while (1) {
        if (!scc8660_init()) {
            printf("[CAM] SCC8660 init success.\r\n");
            break;
        }
        R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);
    }

    /* ---- Init DL1B (VL53L1X) TOF distance sensor ---- */
    {
        uint8_t dl1b_retry = 0;
        while (1) {
            if (!dl1b_init()) {
                printf("[TOF] DL1B init success.\r\n");
                break;
            }
            dl1b_retry++;
            printf("[TOF] DL1B init failed (retry %u), retrying...\r\n",
                   (unsigned)dl1b_retry);
            R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);
        }
    }

    /* ---- Init IPS200 display (landscape 320x240) ---- */
    ips200_set_dir(IPS200_CROSSWISE);
    ips200_init();
    ips200_set_font(IPS200_8X16_FONT);
    ips200_set_color(RGB565_BLACK, RGB565_WHITE);
    ips200_full(RGB565_WHITE);
    ips200_show_string(92, 112, "Initializing...");
    printf("[LCD] IPS200 init success (landscape 320x240)\r\n");
    R_BSP_SoftwareDelay(500, BSP_DELAY_UNITS_MILLISECONDS);

    /* Initialize servos and return to center */
    err = servo0_init();
    handle_error(err, " ** servo0_init FAILED ** \r\n");
    err = servo1_init();
    handle_error(err, " ** servo1_init FAILED ** \r\n");
    err = servo2_init();
    handle_error(err, " ** servo2_init FAILED ** \r\n");
    err = servo_catch_init();
    handle_error(err, " ** servo_catch_init FAILED ** \r\n");

    // servo0_set_angle(0);
    // servo1_set_angle(90);
    // servo2_set_angle(90);

    /* Main loop: capture -> RGB -> YOLO -> state machine */
    while (true) {
        /* Wait for SCC8660 frame complete */
        if (!scc8660_finish_flag) {
            __WFI();
            continue;
        }
        scc8660_finish_flag = false;
        frame_count++;

        /* Restart capture for next frame */
        // g_ceu0.p_api->captureStart(g_ceu0.p_ctrl, (uint8_t *)scc8660_image);

        /* YUYV -> RGB888 (垂直翻转: SCC8660倒装) */
        yuyv_to_rgb888((const uint8_t *)scc8660_image, SCC8660_W, SCC8660_H,
                       g_rgb_buffer, 1);

        /* Geometric distortion correction (径向畸变矫正) */
        geo_correct_rgb888(g_rgb_buffer, g_rgb_corrected,
                           SCC8660_W, SCC8660_H,
                           GEO_K1, GEO_K2, GEO_ASPECT);

        YoloDetectionResult *det_result = YoloApi_RunYolo(&yolo_api, g_rgb_corrected);

        /*---- Convert RGB888 -> RGB565 and display on IPS200 ----*/
        rgb888_to_rgb565_buffer(g_rgb_corrected, g_rgb565_buffer, SCC8660_W * SCC8660_H);
        ips200_show_rgb565_image(0, 0, g_rgb565_buffer, SCC8660_W, SCC8660_H,
                                 240, 240, 0);

        /*---- Draw detection boxes (scale 160->240, factor 1.5), clamp to [0,239] ----*/
        ips200_set_color(RGB565_GREEN, RGB565_BLACK);
        for (int i = 0; i < det_result->count; i++) {
            YoloDetection *d = &det_result->detections[i];
            int x1 = (int)((d->x - d->w / 2.0f) * 1.5f);
            int y1 = (int)((d->y - d->h / 2.0f) * 1.5f);
            int x2 = (int)((d->x + d->w / 2.0f) * 1.5f);
            int y2 = (int)((d->y + d->h / 2.0f) * 1.5f);
            if (x1 < 0) x1 = 0; if (x1 > 239) x1 = 239;
            if (y1 < 0) y1 = 0; if (y1 > 239) y1 = 239;
            if (x2 < 0) x2 = 0; if (x2 > 239) x2 = 239;
            if (y2 < 0) y2 = 0; if (y2 > 239) y2 = 239;
            ips200_draw_line(x1, y1, x2, y1, RGB565_GREEN);
            ips200_draw_line(x2, y1, x2, y2, RGB565_GREEN);
            ips200_draw_line(x2, y2, x1, y2, RGB565_GREEN);
            ips200_draw_line(x1, y2, x1, y1, RGB565_GREEN);
        }

        /*---- Show detection info on right side ----*/
        ips200_set_color(RGB565_WHITE, RGB565_BLACK);
        ips200_show_string(242, 0,   "Detect:");
        ips200_show_uint(242, 16, det_result->count, 2);
        if (det_result->count > 0) {
            ips200_show_string(242, 36, "Score:");
            ips200_show_float(242, 52, det_result->detections[0].score, 1, 3);
        }

        /*---- Read TOF (DL1B) distance if new data available ----*/
        if (dl1b_finsh_flag) {
            dl1b_finsh_flag = false;
            /* Apply -30mm offset correction */
            g_state_machine.tof_distance_mm = (dl1b_distance_mm >= 25) ? (dl1b_distance_mm - 25) : 0;
        }

        /*--- State machine step: controls servos based on detection ---*/
        StateMachine_Step(&g_state_machine, det_result);
    }

    YoloApi_Deinit(&yolo_api);


    /* Wake up 2nd core if this is first core and we are inside a multicore project. */
#if (0 == _RA_CORE) && (1 == BSP_MULTICORE_PROJECT) && !BSP_TZ_NONSECURE_BUILD

#if BSP_TZ_SECURE_BUILD
    /* Take semaphore so 2nd core can clear it */
    R_BSP_IpcSemaphoreTake(&g_core_start_semaphore);
#endif

    R_BSP_SecondaryCoreStart();

#if BSP_TZ_SECURE_BUILD
    /* Wait for 2nd core to start and clear semaphore */
    while(FSP_ERR_IN_USE == R_BSP_IpcSemaphoreTake(&g_core_start_semaphore))
    {
        ;
    }
#endif
#endif

#if (1 == _RA_CORE) && (1 == BSP_MULTICORE_PROJECT) && BSP_TZ_SECURE_BUILD
    /* Signal to 1st core that 2nd core has started */
    R_BSP_IpcSemaphoreGive(&g_core_start_semaphore);
#endif

#if BSP_TZ_SECURE_BUILD
    /* Enter non-secure code */
    R_BSP_NonSecureEnter();
#endif
}

#if BSP_TZ_SECURE_BUILD

FSP_CPP_HEADER
BSP_CMSE_NONSECURE_ENTRY void template_nonsecure_callable ();

/* Trustzone Secure Projects require at least one nonsecure callable function in order to build (Remove this if it is not required to build). */
BSP_CMSE_NONSECURE_ENTRY void template_nonsecure_callable ()
{

}
FSP_CPP_FOOTER

#endif
