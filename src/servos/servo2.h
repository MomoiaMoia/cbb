// servo2.h — GPT5 PWM on P914 (GTIOCB), 180° tilt servo (0°~180°)
#ifndef SERVO2_H_
#define SERVO2_H_

#include <stdint.h>
#include "hal_data.h"

fsp_err_t servo2_init(void);
fsp_err_t servo2_set_angle(uint8_t angle);

#endif /* SERVO2_H_ */
