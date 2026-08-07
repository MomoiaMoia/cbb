#ifndef SERVO3_H_
#define SERVO3_H_

#include <stdint.h>
#include "hal_data.h"

fsp_err_t servo3_init(void);
fsp_err_t servo3_forward(float distance_cm);
fsp_err_t servo3_backward(float distance_cm);

// Pixel width → distance estimation (pinhole model)
// distance_cm = 345.0 / width_pixels
// Returns 0 if width <= 0
float servo3_width_to_distance(float width_pixels);

#endif
