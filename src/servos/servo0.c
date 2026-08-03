// servo0.c — GPT1 PWM on P105 (GTIOCA), 270° pan servo (-135°~+135°)
// Period=20ms(50Hz), PCLK~300MHz, 1µs=300cnts
// 500µs=-135°, 1500µs=0°(neutral), 2500µs=+135°
#include "servo0.h"

// ---- Local helpers ----

// Convert microseconds to timer counts (period = 6,000,000 @ 20 ms)
#define PULSE_US(us)  ((uint32_t)((uint64_t)(us) * 6000000 / 20000))

// Write raw pulse-width count to GPT1 compare register (GTIOCA)
static void servo0_set_pulse(uint32_t pulse_counts)
{
    R_GPT_DutyCycleSet(&g_timer1_ctrl, pulse_counts, GPT_IO_PIN_GTIOCA);
}

// ---- Public API ----

// Open GPT1, set neutral position (0° / 1500µs), start PWM
fsp_err_t servo0_init(void)
{
    fsp_err_t status;

    status = R_GPT_Open(&g_timer1_ctrl, &g_timer1_cfg);
    if (FSP_SUCCESS != status)
    {
        return status;
    }

    // Set 45° before starting to avoid sudden jump
    servo0_set_pulse(PULSE_US(1833));

    status = R_GPT_Start(&g_timer1_ctrl);

    return status;
}

// Set angle (-135°~+135°), linear map: pulse_us = 1500 + angle * 1000 / 135
fsp_err_t servo0_set_angle(int32_t angle)
{
    if (angle < -135 || angle > 135)
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    uint32_t pulse_us = 1500UL + (uint32_t)((int64_t)angle * 1000 / 135);

    servo0_set_pulse(PULSE_US(pulse_us));

    return FSP_SUCCESS;
}
