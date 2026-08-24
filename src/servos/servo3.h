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

// Retract toward base using DL1B feedback: backward(distance), then poll
// DL1B up to 2× to close the gap to 4cm threshold.
// Returns 0 if final reading < 4cm, else returns (reading_mm - 40) / 10.0f
float servo3_backtobase(float distance_cm);

#endif
