#include "servo2.h"

#define PULSE_US(us) ((uint32_t)((uint64_t)(us) * 6000000 / 20000))

static void servo2_set_pulse(uint32_t pulse_counts) {
    R_GPT_DutyCycleSet(&g_timer8_ctrl, pulse_counts, GPT_IO_PIN_GTIOCB);
}

fsp_err_t servo2_init(void) {
    fsp_err_t status;
    status = R_GPT_Open(&g_timer8_ctrl, &g_timer8_cfg);
    if (FSP_SUCCESS != status) {
        return status;
    }
    servo2_set_pulse(PULSE_US(1500));
    status = R_GPT_Start(&g_timer8_ctrl);
    return status;
}

fsp_err_t servo2_set_angle(uint8_t angle) {
    if (angle > 180) {
        return FSP_ERR_INVALID_ARGUMENT;
    }
    uint32_t pulse_us = 500UL + (uint32_t)angle * 2000UL / 180UL;
    servo2_set_pulse(PULSE_US(pulse_us));
    return FSP_SUCCESS;
}
