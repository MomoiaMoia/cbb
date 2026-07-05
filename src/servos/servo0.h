/*******************************************************************************************************************//**
 * @file    servo0.h
 * @brief   Servo0 control module — GPT1 PWM on P105 (GTIOCA)
 *
 * Servo specs:
 *   PWM range:    500 ~ 2500 µsec  →  -135° ~ +135°
 *   Neutral:      1500 µsec (0°)
 *   Frequency:    50 Hz (20 ms period)
 *   Total range:  270° (S20F 270° Position Servo)
 **********************************************************************************************************************/
#ifndef SERVO0_H_
#define SERVO0_H_

#include <stdint.h>
#include "hal_data.h"

/**
 * Initialize servo0.
 * Opens GPT1 timer and starts PWM output at neutral (0°).
 * @return FSP_SUCCESS on success, or error code.
 */
fsp_err_t servo0_init(void);

/**
 * Move servo to the specified angle.
 * @param[in] angle  Target angle in degrees (-135 ~ +135).
 * @return FSP_SUCCESS on success, FSP_ERR_INVALID_ARGUMENT if angle out of range.
 */
fsp_err_t servo0_set_angle(int32_t angle);

#endif /* SERVO0_H_ */
