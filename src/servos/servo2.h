/*******************************************************************************************************************//**
 * @file    servo2.h
 * @brief   Servo2 control module — GPT5 PWM on P914 (GTIOCB)
 *
 * Servo specs:
 *   PWM range:    500 ~ 2500 µsec  →  0° ~ 180°
 *   Neutral:      1500 µsec (90°)
 *   Frequency:    50 Hz (20 ms period)
 *   Direction:    Counterclockwise
 **********************************************************************************************************************/
#ifndef SERVO2_H_
#define SERVO2_H_

#include <stdint.h>
#include "hal_data.h"

/**
 * Initialize servo2.
 * Opens GPT5 timer and starts PWM output at neutral (90°).
 * @return FSP_SUCCESS on success, or error code.
 */
fsp_err_t servo2_init(void);

/**
 * Move servo to the specified angle.
 * @param[in] angle  Target angle in degrees (0 ~ 180).
 * @return FSP_SUCCESS on success, FSP_ERR_INVALID_ARGUMENT if angle > 180.
 */
fsp_err_t servo2_set_angle(uint8_t angle);

#endif /* SERVO2_H_ */
