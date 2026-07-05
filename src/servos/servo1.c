/*******************************************************************************************************************//**
 * @file    servo1.c
 * @brief   Servo1 control module — GPT1 PWM on P104 (GTIOCB)
 *
 * Hardware:
 *   Timer:  GPT1 (channel 1), TIMER_MODE_PWM — shared with servo0
 *   Period: 6,000,000 counts = 20 ms (50 Hz)
 *   Output: P104 (GTIOCB)
 *   PCLK:   ~300 MHz  →  1 µs = 300 counts
 *
 * Calibration:
 *   500 µs  (150,000 cnts) =   0°
 *   1500 µs (450,000 cnts) =  90° (neutral)
 *   2500 µs (750,000 cnts) = 180°
 *
 * @note    This module shares GPT1 with servo0 (P105, GTIOCA).
 *          servo0_init() must be called first to open and start the timer.
 **********************************************************************************************************************/
#include "servo1.h"

/* ------------------------------------------------------------------ */
/*  Local helpers                                                      */
/* ------------------------------------------------------------------ */

/** Convert microseconds to timer counts (period = 6,000,000 @ 20 ms). */
#define PULSE_US(us)  ((uint32_t)((uint64_t)(us) * 6000000 / 20000))

/** Write a raw pulse-width count to the GPT1 compare register (GTIOCB). */
static void servo1_set_pulse(uint32_t pulse_counts)
{
    R_GPT_DutyCycleSet(&g_timer1_ctrl, pulse_counts, GPT_IO_PIN_GTIOCB);
}

/* ------------------------------------------------------------------ */
/*  Public API                                                         */
/* ------------------------------------------------------------------ */

/*******************************************************************************************************************//**
 * Initialize servo1.
 *
 * Sets the GTIOCB output to neutral (90° / 1500 µs).
 * GPT1 must already be opened and started (e.g. by servo0_init).
 *
 * @retval FSP_SUCCESS  Neutral position set on GTIOCB.
 **********************************************************************************************************************/
fsp_err_t servo1_init(void)
{
    /* Set neutral (90°) — timer is already running from servo0_init */
    servo1_set_pulse(PULSE_US(1500));

    return FSP_SUCCESS;
}

/*******************************************************************************************************************//**
 * Move servo to the specified angle.
 *
 * The angle is linearly mapped to a pulse width:
 *     0° →  500 µs (150,000 cnts)
 *    90° → 1500 µs (450,000 cnts) — neutral
 *   180° → 2500 µs (750,000 cnts)
 *
 * @param[in]  angle  Target angle in degrees (0 ~ 180).
 *
 * @retval FSP_SUCCESS              Pulse width set.
 * @retval FSP_ERR_INVALID_ARGUMENT angle is out of range.
 **********************************************************************************************************************/
fsp_err_t servo1_set_angle(uint8_t angle)
{
    if (angle > 180)
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }

    /* Linear interpolation: pulse_us = 500 + angle * 2000 / 180 */
    uint32_t pulse_us = 500UL + (uint32_t)angle * 2000UL / 180UL;

    servo1_set_pulse(PULSE_US(pulse_us));

    return FSP_SUCCESS;
}
