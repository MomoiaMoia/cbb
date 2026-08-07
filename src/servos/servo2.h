#ifndef SERVO2_H_
#define SERVO2_H_

#include <stdint.h>
#include "hal_data.h"

fsp_err_t servo2_init(void);
fsp_err_t servo2_set_angle(uint8_t angle);

#endif
