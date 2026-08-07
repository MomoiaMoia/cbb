#include "servo0.h"

#define PULSE_US(us)  ((uint32_t)((uint64_t)(us) * 300))

static void servo0_set_pulse(uint32_t pulse_counts)
{
    R_GPT_DutyCycleSet(&g_timer1_ctrl, pulse_counts, GPT_IO_PIN_GTIOCA);
}

fsp_err_t servo0_init(void)
{
    fsp_err_t status;
    status = R_GPT_Open(&g_timer1_ctrl, &g_timer1_cfg);
    if (FSP_SUCCESS != status)
    {
        return status;
    }
    servo0_set_pulse(PULSE_US(1833));
    status = R_GPT_Start(&g_timer1_ctrl);
    return status;
}

fsp_err_t servo0_set_angle(int32_t angle)
{
    if (angle < -135 || angle > 135)
    {
        return FSP_ERR_INVALID_ARGUMENT;
    }
    uint32_t pulse_us = 1500UL + (uint32_t)((int64_t)angle * 1000 / 135);
    servo0_set_pulse(PULSE_US(pulse_us));
    return FSP_SUCCESS;
}
