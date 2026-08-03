// servo_catch.c — GPT6 PWM on P600 (GTIOCB), gripper control (夹爪)
// Period=20ms(50Hz), PCLK~300MHz, 1µs=300cnts
// 1400µs=open(张开), 2000µs=close(闭合)
#include "servo_catch.h"

// ---- Local helpers ----

// Convert microseconds to timer counts (period = 6,000,000 @ 20 ms)
#define PULSE_US(us)  ((uint32_t)((uint64_t)(us) * 6000000 / 20000))

// Write raw pulse-width count to GPT6 compare register (GTIOCB)
static void servo_catch_write(uint32_t pulse_counts)
{
    R_GPT_DutyCycleSet(&g_timer6_ctrl, pulse_counts, GPT_IO_PIN_GTIOCB);
}

// ---- Public API ----

// Open GPT6, set open position (1400µs), start PWM
fsp_err_t servo_catch_init(void)
{
    fsp_err_t status;

    status = R_GPT_Open(&g_timer6_ctrl, &g_timer6_cfg);
    if (FSP_SUCCESS != status)
    {
        return status;
    }

    // Set open position before starting to avoid sudden jump
    servo_catch_write(PULSE_US(1400));

    status = R_GPT_Start(&g_timer6_ctrl);

    return status;
}

// Set gripper pulse width directly (500~2500µs)
fsp_err_t servo_catch_set_pulse(uint32_t pulse_us)
{
    if (pulse_us < 500 || pulse_us > 2500)
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    servo_catch_write(PULSE_US(pulse_us));

    return FSP_SUCCESS;
}

// Open gripper (张开), fixed at 1400µs
fsp_err_t servo_catch_open(void)
{
    servo_catch_set_pulse(1400);

    return FSP_SUCCESS;
}

// Close gripper (闭合), fixed at 2000µs
fsp_err_t servo_catch_close(void)
{
    servo_catch_set_pulse(2000);

    return FSP_SUCCESS;
}

// Gradually close gripper (缓慢闭合), ramp 1300→2000µs in 50µs steps
void servo_catch_close_slow(uint32_t duration_ms)
{
    const uint32_t start_us = 1300;
    const uint32_t end_us   = 2000;
    const uint32_t step_us  = 50;
    uint32_t steps = (end_us - start_us) / step_us;
    uint32_t step_delay = duration_ms / steps;
    if (step_delay < 1) step_delay = 1;

    for (uint32_t p = start_us; p <= end_us; p += step_us)
    {
        servo_catch_set_pulse(p);
        R_BSP_SoftwareDelay(step_delay, BSP_DELAY_UNITS_MILLISECONDS);
    }
    servo_catch_set_pulse(end_us);
}
