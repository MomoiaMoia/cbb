// rf433.c — 433MHz OOK/ASK receiver signal detection (~10ms window)
// 200 samples at 50µs interval; any HIGH = signal detected
#include "rf433.h"
#include "hal_data.h"

#define SCAN_WINDOW_US 10000
#define SCAN_INTERVAL_US 50
#define SCAN_SAMPLES (SCAN_WINDOW_US / SCAN_INTERVAL_US)  // 200

// Reserved for future extension (e.g. pull-up config)
void rf433_init(void) {
}

// Dense-sample P707 pin over 10ms window, return 1 on first HIGH
int rf433_scan707(void) {
    bsp_io_level_t level;

    for (int i = 0; i < SCAN_SAMPLES; i++) {
        R_IOPORT_PinRead(&g_ioport_ctrl, BSP_IO_PORT_07_PIN_07, &level);

        if (BSP_IO_LEVEL_HIGH == level) {
            return 1;
        }

        R_BSP_SoftwareDelay(SCAN_INTERVAL_US, BSP_DELAY_UNITS_MICROSECONDS);
    }

    return 0;
}

// Dense-sample P705 pin over 10ms window, return 1 on first HIGH
int rf433_scan705(void) {
    bsp_io_level_t level;

    for (int i = 0; i < SCAN_SAMPLES; i++) {
        R_IOPORT_PinRead(&g_ioport_ctrl, BSP_IO_PORT_07_PIN_05, &level);

        if (BSP_IO_LEVEL_HIGH == level) {
            return 1;
        }

        R_BSP_SoftwareDelay(SCAN_INTERVAL_US, BSP_DELAY_UNITS_MICROSECONDS);
    }

    return 0;
}
