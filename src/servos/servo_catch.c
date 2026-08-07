#include "servo_catch.h"

#define PULSE_US(us) ((uint32_t)((uint64_t)(us) * 300))

#define ANGLE_MIN 0
#define ANGLE_MAX 135
#define SERVO_RANGE 270
#define PULSE_MIN 500
#define PULSE_MAX 2500

static void servo_catch_write(uint32_t pulse_counts) {
    R_GPT_DutyCycleSet(&g_timer7_ctrl, pulse_counts, GPT_IO_PIN_GTIOCA);
}

static uint32_t angle_to_pulse(uint8_t angle) {
    return PULSE_MIN + (uint32_t)angle * (PULSE_MAX - PULSE_MIN) / SERVO_RANGE;
}

fsp_err_t servo_catch_init(void) {
    fsp_err_t status;
    status = R_GPT_Open(&g_timer7_ctrl, &g_timer7_cfg);
    if (FSP_SUCCESS != status) {
        return status;
    }
    servo_catch_write(PULSE_US(angle_to_pulse(70)));
    status = R_GPT_Start(&g_timer7_ctrl);
    return status;
}

fsp_err_t servo_catch_lower(void) {
    servo_catch_write(PULSE_US(angle_to_pulse(70)));
    return FSP_SUCCESS;
}

fsp_err_t servo_catch_raise(void) {
    servo_catch_write(PULSE_US(angle_to_pulse(105)));
    return FSP_SUCCESS;
}

fsp_err_t servo_catch_set_angle(uint8_t angle) {
    if (angle < ANGLE_MIN || angle > ANGLE_MAX) {
        return FSP_ERR_INVALID_ARGUMENT;
    }
    servo_catch_write(PULSE_US(angle_to_pulse(angle)));
    return FSP_SUCCESS;
}
