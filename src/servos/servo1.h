// servo1.h — GPT1 PWM on P104 (GTIOCB), 180° shoulder servo (0°~180°)
// Shares GPT1 timer with servo0; servo0_init() must be called first.
#ifndef SERVO1_H_
#define SERVO1_H_

#include <stdint.h>
#include "hal_data.h"

fsp_err_t servo1_init(void);
fsp_err_t servo1_set_angle(uint8_t angle);

#endif /* SERVO1_H_ */
