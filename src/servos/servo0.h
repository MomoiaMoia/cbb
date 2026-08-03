// servo0.h — GPT1 PWM on P105 (GTIOCA), 270° pan servo (-135°~+135°)
#ifndef SERVO0_H_
#define SERVO0_H_

#include <stdint.h>
#include "hal_data.h"

fsp_err_t servo0_init(void);
fsp_err_t servo0_set_angle(int32_t angle);

#endif /* SERVO0_H_ */
