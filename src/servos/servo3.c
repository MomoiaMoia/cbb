#include "servo3.h"
#include "zf_device_dl1b.h"

#define PULSE_US(us) ((uint32_t)((uint64_t)(us) * 300))

#define PULSE_FWD_FAST 2500
#define PULSE_BWD_FAST 500
#define SPEED_FAST_CM_S 9.5f

#define PULSE_FWD_SLOW 2000
#define PULSE_BWD_SLOW 1000
#define SPEED_SLOW_CM_S 6.5f

static void servo3_set_pulse(uint32_t pulse_counts) {
    R_GPT_DutyCycleSet(&g_timer6_ctrl, pulse_counts, GPT_IO_PIN_GTIOCB);
}

static uint32_t distance_to_ms(float distance_cm) {
    return (uint32_t)(distance_cm * 1000.0f / SPEED_SLOW_CM_S + 0.5f);
}

fsp_err_t servo3_init(void) {
    fsp_err_t status;
    status = R_GPT_Open(&g_timer6_ctrl, &g_timer6_cfg);
    if (FSP_SUCCESS != status) {
        return status;
    }
    return status;
}

fsp_err_t servo3_forward(float distance_cm) {
    if (distance_cm <= 0.0f) {
        return FSP_ERR_INVALID_ARGUMENT;
    }
    uint32_t delay_ms = distance_to_ms(distance_cm);
    servo3_set_pulse(PULSE_US(PULSE_FWD_SLOW));
    R_GPT_Start(&g_timer6_ctrl);
    R_BSP_SoftwareDelay(delay_ms, BSP_DELAY_UNITS_MILLISECONDS);
    R_GPT_Stop(&g_timer6_ctrl);
    return FSP_SUCCESS;
}

fsp_err_t servo3_backward(float distance_cm) {
    if (distance_cm <= 0.0f) {
        return FSP_ERR_INVALID_ARGUMENT;
    }
    uint32_t delay_ms = distance_to_ms(distance_cm);
    servo3_set_pulse(PULSE_US(PULSE_BWD_SLOW));
    R_GPT_Start(&g_timer6_ctrl);
    R_BSP_SoftwareDelay(delay_ms, BSP_DELAY_UNITS_MILLISECONDS);
    R_GPT_Stop(&g_timer6_ctrl);
    return FSP_SUCCESS;
}

// Pixel-width-to-distance coefficient (focal_length_px × real_width_cm)
#define WIDTH_TO_DIST_COEFF 345.0f

float servo3_width_to_distance(float width_pixels)
{
    if (width_pixels <= 0.0f)
    {
        return 0.0f;
    }
    return WIDTH_TO_DIST_COEFF / width_pixels;
}

#define BACKTOBASE_THRESH_MM 40  // 4cm
#define BACKTOBASE_MAX_RETRIES 2

// Piecewise TOF distance calibration
static uint16_t calibrate_tof(uint16_t raw_mm)
{
    if (raw_mm <= 60)
    {
        return raw_mm;
    }
    else if (raw_mm <= 100)
    {
        return raw_mm - 10;
    }
    else if (raw_mm <= 200)
    {
        return raw_mm - 20;
    }
    return raw_mm;
}

float servo3_backtobase(float distance_cm)
{
    // 1. Initial retraction
    if (distance_cm > 0.0f)
    {
        servo3_backward(distance_cm);
    }

    uint16_t calibrated = 0;

    for (int retry = 0; retry < BACKTOBASE_MAX_RETRIES; retry++)
    {
        // Poll DL1B until a new reading arrives
        dl1b_finsh_flag = 0;
        while (!dl1b_finsh_flag)
        {
            // busy-wait for TOF measurement
        }
        dl1b_finsh_flag = 0;

        calibrated = calibrate_tof(dl1b_distance_mm);

        // Within threshold → done
        if (calibrated < BACKTOBASE_THRESH_MM)
        {
            return 0.0f;
        }

        // Still too far — retract the excess
        float excess_cm = (float)(calibrated - BACKTOBASE_THRESH_MM) / 10.0f;
        servo3_backward(excess_cm);
    }

    // Final check after all retries
    if (calibrated < BACKTOBASE_THRESH_MM)
    {
        return 0.0f;
    }
    return (float)(calibrated - BACKTOBASE_THRESH_MM) / 10.0f;
}
