#ifndef SERVO0_H_
#define SERVO0_H_

#include <stdint.h>
#include "hal_data.h"

fsp_err_t servo0_init(void);
fsp_err_t servo0_set_angle(int32_t angle);

#endif
