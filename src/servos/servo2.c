/*******************************************************************************************************************//**
 * @file    servo2.c
 * @brief   Servo2 control module — GPT5 PWM on P914 (GTIOCB)
 *
 * Hardware:
 *   Timer:  GPT5 (channel 5), TIMER_MODE_PWM
 *   Period: 6,000,000 counts = 20 ms (50 Hz)
 *   Output: P914 (GTIOCB)
 *   PCLK:   ~300 MHz  →  1 µs = 300 counts
 **********************************************************************************************************************/
#include "servo2.h"

/* ------------------------------------------------------------------ */
/*  Local helpers                                                      */
/* ------------------------------------------------------------------ */

/** Convert microseconds to timer counts (period = 6,000,000 @ 20 ms). */
#define PULSE_US(us)  ((uint32_t)((uint64_t)(us) * 6000000 / 20000))

/**
 * Write a raw pulse-width count to the GPT5 compare register (GTIOCB).
 */
static void servo2_set_pulse(uint32_t pulse_counts)
{
    R_GPT_DutyCycleSet(&g_timer5_ctrl, pulse_counts, GPT_IO_PIN_GTIOCB);
}

/* ------------------------------------------------------------------ */
/*  Public API                                                         */
/* ------------------------------------------------------------------ */

/*******************************************************************************************************************//**
 * Initialize servo2.
 *
 * Opens GPT5, sets the initial position to neutral (90° / 1500 µs),
 * then starts PWM output.
 *
 * @retval FSP_SUCCESS         GPT5 opened and PWM started.
 * @retval FSP_ERR_IN_USE      GPT5 already open.
 * @retval FSP_ERR_ALREADY_OPEN  GPT5 already open.
 **********************************************************************************************************************/
fsp_err_t servo2_init(void)
{
    fsp_err_t status;

    /* Open GPT5 timer */
    status = R_GPT_Open(&g_timer5_ctrl, &g_timer5_cfg);
    if (FSP_SUCCESS != status)
    {
        return status;
    }

    /* Set neutral (90°) before starting to avoid a sudden jump */
    servo2_set_pulse(PULSE_US(1500));

    /* Start PWM output */
    status = R_GPT_Start(&g_timer5_ctrl);

    return status;
}

/*******************************************************************************************************************//**
 * Move servo to the specified angle.
 *
 * The angle is linearly mapped to a pulse width:
 *   0°   →  500 µs
 *   90°  → 1500 µs (neutral)
 *   180° → 2500 µs
 *
 * @param[in]  angle  Target angle in degrees (0 ~ 180).
 *
 * @retval FSP_SUCCESS              Pulse width set.
 * @retval FSP_ERR_INVALID_ARGUMENT angle is out of range.
 **********************************************************************************************************************/
fsp_err_t servo2_set_angle(uint8_t angle)
{
    if (angle > 180)
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    /* Linear interpolation: pulse_us = 500 + angle * (2500-500) / 180 */
    uint32_t pulse_us = 500UL + (uint32_t)angle * 2000UL / 180UL;

    servo2_set_pulse(PULSE_US(pulse_us));

    return FSP_SUCCESS;
}
