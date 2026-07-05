/*******************************************************************************************************************//**
 * @file    servo_catch.h
 * @brief   Gripper (夹爪) control module — GPT6 PWM on P600 (GTIOCB)
 *
 * Gripper specs:
 *   PWM range:    500 ~ 2500 µsec
 *   Open:         1000 µs
 *   Close:        1800 µs
 *   Frequency:    50 Hz (20 ms period)
 **********************************************************************************************************************/
#ifndef SERVO_CATCH_H_
#define SERVO_CATCH_H_

#include <stdint.h>
#include "hal_data.h"

/**
 * Initialize the gripper.
 * Opens GPT6 timer, sets open position (1000 µs), and starts PWM output.
 * @return FSP_SUCCESS on success, or error code.
 */
fsp_err_t servo_catch_init(void);

/**
 * Open the gripper (张开).
 * Fixed at 1000 µs.
 * @return FSP_SUCCESS on success.
 */
fsp_err_t servo_catch_open(void);

/**
 * Set the gripper pulse width directly (full range).
 * @param[in] pulse_us  Pulse width in microseconds (500 ~ 2500).
 * @return FSP_SUCCESS on success, FSP_ERR_INVALID_ARGUMENT if out of range.
 */
fsp_err_t servo_catch_set_pulse(uint32_t pulse_us);

/**
 * Close the gripper (闭合).
 * Fixed at 1800 µs.
 * @return FSP_SUCCESS on success.
 */
fsp_err_t servo_catch_close(void);

/**
 * Close the gripper gradually (缓慢闭合).
 * Ramps from open (1000 µs) to close (1800 µs) in small steps
 * over the specified duration.
 * @param[in] duration_ms  Total time for the closing motion (ms).
 */
void servo_catch_close_slow(uint32_t duration_ms);

#endif /* SERVO_CATCH_H_ */
