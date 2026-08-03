// servo2.c — GPT5 PWM on P914 (GTIOCB), 180° tilt servo (0°~180°)
// Period=20ms(50Hz), PCLK~300MHz, 1µs=300cnts
// 500µs=0°, 1500µs=90°(neutral), 2500µs=180°
#include "servo2.h"

// ---- Local helpers ----

// Convert microseconds to timer counts (period = 6,000,000 @ 20 ms)
#define PULSE_US(us)  ((uint32_t)((uint64_t)(us) * 6000000 / 20000))

// Write raw pulse-width count to GPT5 compare register (GTIOCB)
static void servo2_set_pulse(uint32_t pulse_counts)
{
    R_GPT_DutyCycleSet(&g_timer5_ctrl, pulse_counts, GPT_IO_PIN_GTIOCB);
}

// ---- Public API ----

// Open GPT5, set neutral position (90° / 1500µs), start PWM
fsp_err_t servo2_init(void)
{
    fsp_err_t status;

    status = R_GPT_Open(&g_timer5_ctrl, &g_timer5_cfg);
    if (FSP_SUCCESS != status)
    {
        return status;
    }

    // Set neutral before starting to avoid sudden jump
    servo2_set_pulse(PULSE_US(1500));

    status = R_GPT_Start(&g_timer5_ctrl);

    return status;
}

// Set angle (0°~180°), linear map: pulse_us = 500 + angle * 2000 / 180
fsp_err_t servo2_set_angle(uint8_t angle)
{
    if (angle > 180)
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    uint32_t pulse_us = 500UL + (uint32_t)angle * 2000UL / 180UL;

    servo2_set_pulse(PULSE_US(pulse_us));

    return FSP_SUCCESS;
}
