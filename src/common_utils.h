/***********************************************************************************************************************
 * File Name    : common_utils.h
 * Description  : Contains macros, data structures and functions used  common to the EP
 ***********************************************************************************************************************/
/***********************************************************************************************************************
 * Copyright (c) 2020 - 2024 Renesas Electronics Corporation and/or its affiliates
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ***********************************************************************************************************************/

#ifndef COMMON_UTILS_H_
#define COMMON_UTILS_H_

/* generic headers */
#include "hal_data.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* SEGGER RTT and error related headers */
#include "SEGGER_RTT/SEGGER_RTT.h"

#define BIT_SHIFT_8 (8u)
#define SIZE_64 (64u)

#define LVL_ERR (1u) /* error conditions   */

#define RESET_VALUE (0x00)

#define EP_VERSION ("1.0")
#define MODULE_NAME "r_ceu"
#define BANNER_INFO "\r\n********************************************************************************"           \
                    "\r\n*   Renesas FSP Example Project for " MODULE_NAME " Module                               *" \
                    "\r\n*   Example Project Version %s                                                *"            \
                    "\r\n*   Flex Software Pack Version  %d.%d.%d                                          *"        \
                    "\r\n********************************************************************************"           \
                    "\r\nRefer to readme.txt file for more details on Example Project and"                           \
                    "\r\nFSP User's Manual for more information about " MODULE_NAME " driver\r\n"

#define EP_INFO "\r\nThe Example Project demonstrates the basic functionality of CEU driver on Renesas \r\n"                \
                "RA MCUs based on FSP. On successful initialization of CEU and camera setup,\r\n"                           \
                "user is requested to select memory (1. SDRAM and 2. SRAM) to save image.\r\n"                              \
                "Type '1' and enter to select 'SDRAM' to choose SXGA (1280 x 960) format and store the image in SDRAM.\r\n" \
                "Type '2' and enter to select 'SRAM' to choose VGA (640 x 480) format and store the image in SRAM.\r\n"     \
                "The captured image can be viewed in e2studio using the 'image viewer'.\r\n\n"

#define SEGGER_INDEX (0U)

#define APP_PRINT(fn_, ...) (SEGGER_RTT_printf(SEGGER_INDEX, (fn_), ##__VA_ARGS__))

#define APP_PRINT_WAIT ({while (SEGGER_RTT_HasDataUp(SEGGER_INDEX)); })

#define APP_ERR_PRINT(fn_, ...) ({                                                                          \
    if (LVL_ERR) {                                                                                          \
        SEGGER_RTT_printf(SEGGER_INDEX, "[ERR] In Function: %s(), %s", __FUNCTION__, (fn_), ##__VA_ARGS__); \
    }                                                                                                       \
})

#define APP_ERR_RETURN(err, fn_, ...) ({                                                                        \
    if (err) {                                                                                                  \
        if (LVL_ERR) {                                                                                          \
            SEGGER_RTT_printf(SEGGER_INDEX, "[ERR] In Function: %s(), %s", __FUNCTION__, (fn_), ##__VA_ARGS__); \
        }                                                                                                       \
        return err;                                                                                             \
    }                                                                                                           \
})

#define APP_ERR_TRAP(err) ({                                                           \
    if (err) {                                                                         \
        SEGGER_RTT_printf(SEGGER_INDEX, "\r\nReturned Error Code: 0x%x  \r\n", (err)); \
        __asm("BKPT #0\n"); /* trap upon the error  */                                 \
    }                                                                                  \
})

#define APP_READ(read_data) (SEGGER_RTT_Read(SEGGER_INDEX, (read_data), sizeof(read_data)))

#define APP_CHECK_DATA (SEGGER_RTT_HasKey())

/*******************************************************************************************************************//**
 * Cycle counter timing utilities (DWT CYCCNT)
 **********************************************************************************************************************/
static inline void cycle_counter_init(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static inline uint32_t cycle_counter_now(void) {
    return DWT->CYCCNT;
}

static inline uint32_t cycles_to_us(uint32_t cycles) {
    return (uint32_t)(((uint64_t)cycles * 1000000ULL) / SystemCoreClock);
}

/*******************************************************************************************************************//**
 * Error handler: print error string, close CEU, then trap
 **********************************************************************************************************************/
static inline void handle_error(fsp_err_t err, const char *err_str) {
    if (FSP_SUCCESS != err) {
        APP_PRINT(err_str);

        if (0U != g_ceu0_ctrl.open) {
            if (FSP_SUCCESS != R_CEU_Close(&g_ceu0_ctrl)) {
                APP_ERR_PRINT("R_CEU_Close FAILED\r\n");
            }
        }

        APP_ERR_TRAP(err);
    }
}

#endif /* COMMON_UTILS_H_ */
