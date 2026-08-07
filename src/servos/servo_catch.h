#ifndef SERVO_CATCH_H_
#define SERVO_CATCH_H_

#include <stdint.h>
#include "hal_data.h"

fsp_err_t servo_catch_init(void);
fsp_err_t servo_catch_lower(void);
fsp_err_t servo_catch_raise(void);
fsp_err_t servo_catch_set_angle(uint8_t angle);

#endif
