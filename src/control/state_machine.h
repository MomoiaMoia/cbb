// state_machine.h — Blueberry-picking state machine
// States: INIT → SCAN_SERVO2 → SCAN_SERVO0 → CONFIRM → CALIBRATE → CENTERED → APPROACH → CATCHING → HALTING
// Servo0=pan(-135~+135), Servo2=tilt(0~180), Servo3=telescopic(0~15cm)
// Servo1 (shoulder) fixed at 90° by hal init
#ifndef STATE_MACHINE_H_
#define STATE_MACHINE_H_

#include "yolo/yolo_api.h"
#include <stdbool.h>
#include <stdint.h>

// State enumeration
typedef enum {
    STATE_INIT = 0,    // Hardware init, set initial scan pose
    STATE_SCAN_SERVO2, // Vertical sweep 110°→50°
    STATE_SCAN_SERVO0, // X-shaped diagonal scan
    STATE_CONFIRM,     // N consecutive frames with detection
    STATE_CALIBRATE,   // P-controller centering via servo0 & servo2
    STATE_CENTERED,    // Target centered, monitor drift/loss
    STATE_APPROACH,    // Move arm forward toward target
    STATE_CATCHING,    // Scoop → retract → shake → lower
    STATE_HALTING      // Restore s0/s2 to pre-catch pose
} SystemState;

// Return human-readable state name (debug)
const char *StateMachine_StateName(SystemState s);

// State machine context
typedef struct {
    SystemState state;

    // Scanning
    int32_t scan_angle; // Current sweep angle
    int32_t scan_step;  // Degrees per step
    uint8_t scan_retry; // Lost-target retry level (0=keep, 1=center+V, 2=X-scan)
    uint8_t scan_phase; // SCAN_SERVO0 sub-phase: 0=↘, 1=↗, 2=↙

    // Confirmation (15-frame window, ≥2 matches within 30px)
    float confirm_anchor_cx, confirm_anchor_cy, confirm_anchor_score, confirm_anchor_width;
    uint8_t confirm_frame_count;
    uint8_t confirm_success_count;

    // Target tracking
    float track_cx, track_cy;

    // Loss tracking
    uint8_t lose_counter;

    // Current servo positions
    int32_t cur_servo0;   // -135…+135
    uint8_t cur_servo2;   // 0…180
    float   cur_servo3;   // telescopic extension distance (cm), 0=retracted, 15=max

    // Pre-catch recorded positions
    int32_t pre_catch_servo0;
    uint8_t pre_catch_servo2;
    float   pre_catch_servo3;

    // Approach
    float   approach_target_width; // bb_width saved from CENTERED

    // Calibration sliding window
    float calib_cx_buf[15], calib_cy_buf[15];
    uint8_t calib_buf_idx, calib_sample_count;

    // P-controller parameters
    float kp_pan, kp_tilt;        // Proportional gains
    float deadband_x, deadband_y; // |offset| below this = centred (px)
} StateMachine;

// Initialize state machine to defaults and STATE_INIT
void StateMachine_Init(StateMachine *sm);

// Execute one state-machine step per frame; moves servos as required
void StateMachine_Step(StateMachine *sm, const YoloDetectionResult *det);

#endif /* STATE_MACHINE_H_ */
