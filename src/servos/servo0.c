/*******************************************************************************************************************//**
 * @file    servo0.c
 * @brief   Servo0 control module — GPT1 PWM on P105 (GTIOCA)
 *
 * Hardware:
 *   Timer:  GPT1 (channel 1), TIMER_MODE_PWM
 *   Period: 6,000,000 counts = 20 ms (50 Hz)
 *   Output: P105 (GTIOCA)
 *   PCLK:   ~300 MHz  →  1 µs = 300 counts
 *
 * Calibration:
 *   500 µs  (150,000 cnts) = -135° (CW limit)
 *   1500 µs (450,000 cnts) =   0° (center)
 *   2500 µs (750,000 cnts) = +135° (CCW limit)
 *   Total range: 270°
 **********************************************************************************************************************/
#include "servo0.h"

/* ------------------------------------------------------------------ */
/*  Local helpers                                                      */
/* ------------------------------------------------------------------ */

/** Convert microseconds to timer counts (period = 6,000,000 @ 20 ms). */
#define PULSE_US(us)  ((uint32_t)((uint64_t)(us) * 6000000 / 20000))

/** Write a raw pulse-width count to the GPT1 compare register (GTIOCA). */
static void servo0_set_pulse(uint32_t pulse_counts)
{
    R_GPT_DutyCycleSet(&g_timer1_ctrl, pulse_counts, GPT_IO_PIN_GTIOCA);
}

/* ------------------------------------------------------------------ */
/*  Public API                                                         */
/* ------------------------------------------------------------------ */

/*******************************************************************************************************************//**
 * Initialize servo0.
 *
 * Opens GPT1, sets the initial position to neutral (0° / 1500 µs),
 * then starts PWM output.
 *
 * @retval FSP_SUCCESS         GPT1 opened and PWM started.
 * @retval FSP_ERR_IN_USE      GPT1 already open.
 * @retval FSP_ERR_ALREADY_OPEN  GPT1 already open.
 **********************************************************************************************************************/
fsp_err_t servo0_init(void)
{
    fsp_err_t status;

    /* Open GPT1 timer */
    status = R_GPT_Open(&g_timer1_ctrl, &g_timer1_cfg);
    if (FSP_SUCCESS != status)
    {
        return status;
    }

    /* Set 45° before starting to avoid a sudden jump */
    servo0_set_pulse(PULSE_US(1833));

    /* Start PWM output */
    status = R_GPT_Start(&g_timer1_ctrl);

    return status;
}

/*******************************************************************************************************************//**
 * Move servo to the specified angle.
 *
 * The angle is linearly mapped to a pulse width:
 *   -135° →  500 µs (150,000 cnts)
 *      0° → 1500 µs (450,000 cnts) — neutral
 *   +135° → 2500 µs (750,000 cnts)
 *
 * @param[in]  angle  Target angle in degrees (-135 ~ +135).
 *
 * @retval FSP_SUCCESS              Pulse width set.
 * @retval FSP_ERR_INVALID_ARGUMENT angle is out of range.
 **********************************************************************************************************************/
fsp_err_t servo0_set_angle(int32_t angle)
{
    if (angle < -135 || angle > 135)
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    /* Linear interpolation: pulse_us = 1500 + angle * 1000 / 135 */
    uint32_t pulse_us = 1500UL + (uint32_t)((int64_t)angle * 1000 / 135);

    servo0_set_pulse(PULSE_US(pulse_us));

    return FSP_SUCCESS;
}
