// servo_catch.h — GPT6 PWM on P600 (GTIOCB), gripper control
// Pulse: 1400µs=open, 2000µs=close, 50Hz PWM
#ifndef SERVO_CATCH_H_
#define SERVO_CATCH_H_

#include <stdint.h>
#include "hal_data.h"

fsp_err_t servo_catch_init(void);
fsp_err_t servo_catch_open(void);
fsp_err_t servo_catch_set_pulse(uint32_t pulse_us);
fsp_err_t servo_catch_close(void);
void servo_catch_close_slow(uint32_t duration_ms);

#endif /* SERVO_CATCH_H_ */
