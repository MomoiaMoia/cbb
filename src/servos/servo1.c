#include "servo1.h"

#define PULSE_US(us)  ((uint32_t)((uint64_t)(us) * 300))

static void servo1_set_pulse(uint32_t pulse_counts)
{
    R_GPT_DutyCycleSet(&g_timer1_ctrl, pulse_counts, GPT_IO_PIN_GTIOCB);
}

fsp_err_t servo1_init(void)
{
    servo1_set_pulse(PULSE_US(1500));
    return FSP_SUCCESS;
}
