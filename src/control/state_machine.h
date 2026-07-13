/*******************************************************************************************************************//**
 * @file    state_machine.h
 * @brief   Blueberry-picking state machine
 *
 * States:
 *   INIT          → set initial pose, then → SCAN_SERVO2
 *   SCAN_SERVO2   → vertical sweep searching for blueberry
 *   SCAN_SERVO0   → X-shaped diagonal scan
 *   CONFIRM       → windowed detection confirmation
 *   CALIBRATE     → P-controller centering via servo0 (pan) & servo2 (tilt)
 *   CENTERED      → target centered, monitor for drift / loss
 *   APPROACH      → move arm forward until close enough
 *   CATCHING      → pickup sequence: close gripper, lift arm, rotate base
 *   HALTING       → restore arm to scanning pose and stop
 *
 * Scan retry ladder (only mode):
 *   0 — keep current servo0, vertical sweep
 *   1 — reset servo0 to centre, vertical sweep
 *   2 — X-shaped diagonal scan
 *   After level 2 → back to level 1 (cycle)
 **********************************************************************************************************************/
#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

#include <stdint.h>
#include <stdbool.h>
#include "yolo/yolo_api.h"

/* ------------------------------------------------------------------ */
/*  State enumeration                                                  */
/* ------------------------------------------------------------------ */
typedef enum {
    STATE_INIT = 0,         // Hardware init → set initial scan pose
    STATE_SCAN_SERVO2,      // Vertical sweep
    STATE_SCAN_SERVO0,      // Horizontal sweep: servo0 -45° → +135°
    STATE_CONFIRM,          // Require N consecutive frames with detection
    STATE_CALIBRATE,        // Centering via P-controller
    STATE_CENTERED,         // Target centered — monitor for drift/loss
    STATE_APPROACH,         // Move arm forward toward target
    STATE_CATCHING,         // Pickup: close gripper, lift, rotate, drop
    STATE_HALTING           // Restore scanning pose and stop
} SystemState;

/* Return a human-readable name for a state (debug). */
const char* StateMachine_StateName(SystemState s);

/* ------------------------------------------------------------------ */
/*  State machine context                                              */
/* ------------------------------------------------------------------ */
typedef struct {
    SystemState state;              // Current state

    /* ---- Scanning ---- */
    int32_t     scan_angle;         // Current sweep angle
    int32_t     scan_step;          // Degrees per step (5° for servo2, 3° for servo0)
    uint8_t     scan_retry;         // Lost-target retry level (0=keep s0, 1=center+V, 2=H, then cycles 1→2)
    uint8_t     scan_phase;         // SCAN_SERVO0 sub-phase: 0=diag ↘, 1=fast return ↗, 2=diag ↖


    /* ---- Confirmation (15-frame window, need ≥2 matches within 30px) ---- */
    float       confirm_anchor_cx;     // Reference cx from first detection
    float       confirm_anchor_cy;     // Reference cy from first detection
    float       confirm_anchor_score;  // Score of the anchor target
    float       confirm_anchor_width;  // Width (px) of the anchor target
    uint8_t     confirm_frame_count;   // Frames elapsed in CONFIRM (max 30)
    uint8_t     confirm_success_count; // Matches within 30px threshold

    /* ---- Target tracking (prevent switching between multiple targets) ---- */
    float       track_cx;           // Last known cx of the target we're tracking
    float       track_cy;           // Last known cy of the target we're tracking

    /* ---- Loss tracking ---- */
    uint8_t     lose_counter;
    uint8_t     max_lose_frames;    // Default 60

    /* ---- Current servo positions (tracked internally) ---- */
    int32_t     cur_servo0;         // -135 … +135
    uint8_t     cur_servo1;         //   0 … 180
    uint8_t     cur_servo2;         //   0 … 180

    /* ---- Pre-catch recorded positions (snapshot before CATCHING) ---- */
    int32_t     pre_catch_servo0;   // servo0 angle recorded before pickup
    uint8_t     pre_catch_servo1;   // servo1 angle recorded before pickup
    uint8_t     pre_catch_servo2;   // servo2 angle recorded before pickup

    /* ---- Approach ---- */
    uint8_t     approach_step_count; // Steps taken since entering APPROACH


    /* ---- Calibration sliding window average ---- */
    float       calib_cx_buf[15];   // Circular buffer for cx averaging
    float       calib_cy_buf[15];   // Circular buffer for cy averaging
    uint8_t     calib_buf_idx;      // Current write index in circular buffer
    uint8_t     calib_sample_count; // Total samples since entering CALIBRATE

    /* ---- P-controller parameters ---- */
    float       kp_pan;             // Proportional gain for pan  (servo0)
    float       kp_tilt;            // Proportional gain for tilt (servo2)
    float       deadband_x;         // |off_x| below this = centred (default 16.0 px)
    float       deadband_y;         // |off_y| below this = centred (default 16.0 px)

    /* ---- TOF distance sensor (VL53L1X / DL1B) ---- */
    uint16_t    tof_distance_mm;    // Latest distance reading (8192 = no data yet)

} StateMachine;

/* ------------------------------------------------------------------ */
/*  Public API                                                         */
/* ------------------------------------------------------------------ */

/**
 * Initialise state machine to default parameters and STATE_INIT.
 */
void StateMachine_Init(StateMachine *sm);

/**
 * Execute one state-machine step.
 * Must be called once per captured + YOLO-processed frame.
 * Moves servos and busy-waits as required.
 *
 * @param[in,out] sm   State machine context
 * @param[in]     det  Latest YOLO detection result (may be NULL)
 */
void StateMachine_Step(StateMachine *sm, const YoloDetectionResult *det);

#endif /* STATE_MACHINE_H_ */
