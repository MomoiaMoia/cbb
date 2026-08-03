// Copyright (c) 2020 - 2026 Renesas Electronics Corporation and/or its affiliates
// SPDX-License-Identifier: BSD-3-Clause

#include "hal_data.h"

FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event);

FSP_CPP_FOOTER

// Called at various points during startup; sets up pins before main()
void R_BSP_WarmStart (bsp_warm_start_event_t event)
{
    if (BSP_WARM_START_RESET == event)
    {
#if BSP_FEATURE_FLASH_LP_VERSION != 0

        // Enable reading from data flash
        R_FACI_LP->DFLCTL = 1U;

#endif
    }

#if BSP_CFG_OSPI_B_STARTUP_ENABLED && defined(BSP_CFG_OSPI_B_STARTUP_FN)
    if (BSP_WARM_START_POST_CLOCK == event)
    {
        // Setup OSPI_B SiP flash and initialize it
        R_BSP_OspiBInit(BSP_CFG_OSPI_B_STARTUP_FN, true);
    }
#endif

    if (BSP_WARM_START_POST_C == event)
    {
        // C runtime environment and system clocks are setup; configure pins
        R_IOPORT_Open(&IOPORT_CFG_CTRL, &IOPORT_CFG_NAME);

#if BSP_CFG_SDRAM_ENABLED

        // Setup SDRAM and initialize it (must configure pins first)
        R_BSP_SdramInit(true);
#endif
    }
}
