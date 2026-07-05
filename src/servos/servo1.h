/*******************************************************************************************************************//**
 * @file    servo1.h
 * @brief   Servo1 control module — GPT1 PWM on P104 (GTIOCB)
 *
 * Servo specs:
 *   PWM range:    500 ~ 2500 µsec  →  0° ~ 180°
 *   Neutral:      1500 µsec (90°)
 *   Frequency:    50 Hz (20 ms period)
 *   Direction:    Counterclockwise
 *
 * @note    Shares GPT1 timer with servo0 (P105, GTIOCA).
 *          servo0_init() must be called first to open & start GPT1.
 **********************************************************************************************************************/
#ifndef SERVO1_H_
#define SERVO1_H_

#include <stdint.h>
#include "hal_data.h"

/**
 * Initialize servo1.
 * Sets GTIOCB output to neutral (90°). GPT1 must already be opened & started.
 * @return FSP_SUCCESS on success.
 */
fsp_err_t servo1_init(void);

/**
 * Move servo to the specified angle.
 * @param[in] angle  Target angle in degrees (0 ~ 180).
 * @return FSP_SUCCESS on success, FSP_ERR_INVALID_ARGUMENT if angle > 180.
 */
fsp_err_t servo1_set_angle(uint8_t angle);

#endif /* SERVO1_H_ */
