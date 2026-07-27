/* generated common source file - do not edit */
#include "common_data.h"
icu_instance_ctrl_t g_external_irq26_ctrl;

/** External IRQ extended configuration for ICU HAL driver */
const icu_extended_cfg_t g_external_irq26_ext_cfg =
{ .filter_src = EXTERNAL_IRQ_DIGITAL_FILTER_PCLK_DIV, };

const external_irq_cfg_t g_external_irq26_cfg =
{ .channel = 26, .trigger = EXTERNAL_IRQ_TRIG_FALLING, .filter_enable = false, .clock_source_div =
          EXTERNAL_IRQ_CLOCK_SOURCE_DIV_64,
  .p_callback = NULL,
  /** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
  .p_context = (void*) &NULL,
#endif
  .p_extend = (void*) &g_external_irq26_ext_cfg,
  .ipl = (12),
#if defined(VECTOR_NUMBER_ICU_IRQ26)
    .irq                 = VECTOR_NUMBER_ICU_IRQ26,
#else
  .irq = FSP_INVALID_VECTOR,
#endif
        };
/* Instance structure to use this module. */
const external_irq_instance_t g_external_irq26 =
{ .p_ctrl = &g_external_irq26_ctrl, .p_cfg = &g_external_irq26_cfg, .p_api = &g_external_irq_on_icu };
#include "ethosu_driver.h"
struct ethosu_driver g_ethosu0;
rm_ethosu_extended_cfg_t g_rm_ethosu0_ext_cfg =
{ .p_dev = &g_ethosu0, };

rm_ethosu_instance_ctrl_t g_rm_ethosu0_ctrl =
{ .p_ext_cfg = &g_rm_ethosu0_ext_cfg, };

const rm_ethosu_cfg_t g_rm_ethosu0_cfg =
{ .secure_enable = 1, .privilege_enable = 1,
#if defined(VECTOR_NUMBER_NPU_IRQ)
            .irq             = VECTOR_NUMBER_NPU_IRQ,
#else
  .irq = FSP_INVALID_VECTOR,
#endif
  .ipl = (12),
  .p_callback = NULL, .p_context = NULL, };

const rm_ethosu_instance_t g_rm_ethosu0 =
{ .p_ctrl = &g_rm_ethosu0_ctrl, .p_cfg = &g_rm_ethosu0_cfg, .p_api = &g_rm_ethosu_on_npu, };
ioport_instance_ctrl_t g_ioport_ctrl;
const ioport_instance_t g_ioport =
{ .p_api = &g_ioport_on_ioport, .p_ctrl = &g_ioport_ctrl, .p_cfg = &g_bsp_pin_cfg, };
void g_common_init(void)
{
}
