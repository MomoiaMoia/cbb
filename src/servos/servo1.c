// servo1.c — GPT1 PWM on P104 (GTIOCB), 180° shoulder servo (0°~180°)
// Shares GPT1 with servo0; servo0_init() must be called first
// 500µs=0°, 1500µs=90°(neutral), 2500µs=180°
#include "servo1.h"

// ---- Local helpers ----

// Convert microseconds to timer counts (period = 6,000,000 @ 20 ms)
#define PULSE_US(us)  ((uint32_t)((uint64_t)(us) * 6000000 / 20000))

// Write raw pulse-width count to GPT1 compare register (GTIOCB)
static void servo1_set_pulse(uint32_t pulse_counts)
{
    R_GPT_DutyCycleSet(&g_timer1_ctrl, pulse_counts, GPT_IO_PIN_GTIOCB);
}

// ---- Public API ----

// Set GTIOCB output to neutral (90° / 1500µs); GPT1 must already be started
fsp_err_t servo1_init(void)
{
    servo1_set_pulse(PULSE_US(1500));

    return FSP_SUCCESS;
}

// Set angle (0°~180°), linear map: pulse_us = 500 + angle * 2000 / 180
fsp_err_t servo1_set_angle(uint8_t angle)
{
    if (angle > 180)
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    uint32_t pulse_us = 500UL + (uint32_t)angle * 2000UL / 180UL;

    servo1_set_pulse(PULSE_US(pulse_us));

    return FSP_SUCCESS;
}
