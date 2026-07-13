/*******************************************************************************************************************//**
 * @file    servo_catch.c
 * @brief   Gripper (夹爪) control module — GPT6 PWM on P600 (GTIOCB)
 *
 * Hardware:
 *   Timer:  GPT6 (channel 6), TIMER_MODE_PWM
 *   Period: 6,000,000 counts = 20 ms (50 Hz)
 *   Output: P600 (GTIOCB)
 *   PCLK:   ~300 MHz  →  1 µs = 300 counts
 *
 * Calibration:
 *    500 µs — one direction limit
 *   1400 µs — open (张开)
 *   2000 µs — close (闭合)
 *   2500 µs — other direction limit
 **********************************************************************************************************************/
#include "servo_catch.h"

/* ------------------------------------------------------------------ */
/*  Local helpers                                                      */
/* ------------------------------------------------------------------ */

/** Convert microseconds to timer counts (period = 6,000,000 @ 20 ms). */
#define PULSE_US(us)  ((uint32_t)((uint64_t)(us) * 6000000 / 20000))

/** Write a raw pulse-width count to the GPT6 compare register (GTIOCB). */
static void servo_catch_write(uint32_t pulse_counts)
{
    R_GPT_DutyCycleSet(&g_timer6_ctrl, pulse_counts, GPT_IO_PIN_GTIOCB);
}

/* ------------------------------------------------------------------ */
/*  Public API                                                         */
/* ------------------------------------------------------------------ */

/*******************************************************************************************************************//**
 * Initialize the gripper.
 *
 * Opens GPT6, sets the initial position to open (700 µs),
 * then starts PWM output.
 *
 * @retval FSP_SUCCESS         GPT6 opened and PWM started.
 * @retval FSP_ERR_IN_USE      GPT6 already open.
 * @retval FSP_ERR_ALREADY_OPEN  GPT6 already open.
 **********************************************************************************************************************/
fsp_err_t servo_catch_init(void)
{
    fsp_err_t status;

    /* Open GPT6 timer */
    status = R_GPT_Open(&g_timer6_ctrl, &g_timer6_cfg);
    if (FSP_SUCCESS != status)
    {
        return status;
    }

    /* Set open position (1400 µs) before starting to avoid a sudden jump */
    servo_catch_write(PULSE_US(1400));

    /* Start PWM output */
    status = R_GPT_Start(&g_timer6_ctrl);

    return status;
}

/*******************************************************************************************************************//**
 * Set the gripper pulse width directly.
 *
 * @param[in]  pulse_us  Pulse width in microseconds (500 ~ 2500).
 *
 * @retval FSP_SUCCESS              Pulse width set.
 * @retval FSP_ERR_INVALID_ARGUMENT pulse_us < 500 or > 2500.
 **********************************************************************************************************************/
fsp_err_t servo_catch_set_pulse(uint32_t pulse_us)
{
    if (pulse_us < 500 || pulse_us > 2500)
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    servo_catch_write(PULSE_US(pulse_us));

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Open the gripper (张开).
 *
 * Fixed at 1100 µs.
 *
 * @retval FSP_SUCCESS  Gripper opened.
 **********************************************************************************************************************/
fsp_err_t servo_catch_open(void)
{
    servo_catch_set_pulse(1400);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Close the gripper (闭合).
 *
 * Fixed at 2000 µs.
 *
 * @retval FSP_SUCCESS  Gripper closed.
 **********************************************************************************************************************/
fsp_err_t servo_catch_close(void)
{
    servo_catch_set_pulse(2000);

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Close the gripper gradually (缓慢闭合).
 *
 * Ramps from 1400 µs to 2000 µs in 50 µs steps.
 *
 * @param[in] duration_ms  Total time for the closing motion (ms).
 **********************************************************************************************************************/
void servo_catch_close_slow(uint32_t duration_ms)
{
    const uint32_t start_us = 1400;
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
    /* Ensure fully closed */
    servo_catch_set_pulse(end_us);
}
