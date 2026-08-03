// state_machine.c — Blueberry-picking state machine implementation
// Servo0=pan(-135~+135), Servo1=shoulder(0~180), Servo2=tilt(0~180)
// cx offset→servo0(pan), cy offset→servo2(tilt); target=(80,95) in 160x160 image
#include "state_machine.h"

#include "common_utils.h"
#include "hal_data.h"
#include "servos/servo0.h"
#include "servos/servo1.h"
#include "servos/servo2.h"
#include "servos/servo_catch.h"

#include <math.h>

// Debug output routing
#define SM_DBG_USE_RTT 0

#if SM_DBG_USE_RTT
#define SM_PRINT(fmt, ...) APP_PRINT(fmt, ##__VA_ARGS__)
#else
#define SM_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif

// Calibration target (pixel coords, 0-160)
#define CX_TARGET 80.0f
#define CY_TARGET 95.0f

// P-controller gains & deadband
#define KP_PAN 0.20f
#define KP_TILT 0.20f
#define DEADBAND_X 16.0f
#define DEADBAND_Y 16.0f
#define MAX_LOSE_FRAMES 30u

// Sliding-window size & CATCHING width threshold
#define CALIB_WIN_SIZE 2u
#define CATCH_WIDTH_THRESH 43.0f

// Clamp helpers
static int32_t clamp_i32(int32_t val, int32_t lo, int32_t hi) {
    if (val < lo)
        return lo;
    if (val > hi)
        return hi;
    return val;
}

// Clamp int32_t to [lo, hi] and return as uint8_t
static uint8_t clamp_u8(int32_t val, int32_t lo, int32_t hi) {
    if (val < lo)
        return (uint8_t)lo;
    if (val > hi)
        return (uint8_t)hi;
    return (uint8_t)val;
}

// Blocking delay using Renesas FSP software delay
static void delay_ms(uint32_t ms) {
    if (ms == 0) return;
    R_BSP_SoftwareDelay(ms, BSP_DELAY_UNITS_MILLISECONDS);
}

// Step delay (ms) per degree — speed levels
#define SLOW_STEP_MS 30
#define NORMAL_STEP_MS 15
#define FAST_STEP_MS 5

// Smooth 1°/step servo0 movement
static void smooth_move_servo0(int32_t from, int32_t to, uint32_t step_ms) {
    if (from == to) {
        servo0_set_angle(to);
        return;
    }
    int32_t step = (to > from) ? 1 : -1;
    int32_t a = from;
    while (a != to) {
        a += step;
        servo0_set_angle(a);
        delay_ms(step_ms);
    }
}

static void smooth_move_servo1(uint8_t from, uint8_t to, uint32_t step_ms) {
    if (from == to) {
        servo1_set_angle(to);
        return;
    }
    int32_t step = (to > from) ? 1 : -1;
    int32_t a = (int32_t)from;
    while (a != (int32_t)to) {
        a += step;
        servo1_set_angle((uint8_t)a);
        delay_ms(step_ms);
    }
}

static void smooth_move_servo2(uint8_t from, uint8_t to, uint32_t step_ms) {
    if (from == to) {
        servo2_set_angle(to);
        return;
    }
    int32_t step = (to > from) ? 1 : -1;
    int32_t a = (int32_t)from;
    while (a != (int32_t)to) {
        a += step;
        servo2_set_angle((uint8_t)a);
        delay_ms(step_ms);
    }
}

// ---- Scan retry ladder ----

// Scan retry levels: 0=keep current s0+V sweep, 1=centre+V, 2=X-scan
static void prv_enter_scan_retry(StateMachine *sm) {
    switch (sm->scan_retry) {
    case 0:
        // Keep current servo0, start vertical sweep 110→50
        smooth_move_servo2(sm->cur_servo2, 110, NORMAL_STEP_MS);
        sm->cur_servo2 = 110;
        sm->scan_angle = 110;
        sm->scan_step = 5;
        sm->state = STATE_SCAN_SERVO2;
        SM_PRINT("[SM] retry=0  servo0=%ld  SCAN_SERVO2 110->50\r\n", (long)sm->cur_servo0);
        break;

    case 1:
        // Reset to centre, restore servo1 vertical, vertical sweep 110→50
        smooth_move_servo0(sm->cur_servo0, 45, NORMAL_STEP_MS);
        sm->cur_servo0 = 45;
        smooth_move_servo1(sm->cur_servo1, 90, NORMAL_STEP_MS);
        sm->cur_servo1 = 90;
        smooth_move_servo2(sm->cur_servo2, 110, NORMAL_STEP_MS);
        sm->cur_servo2 = 110;
        sm->scan_angle = 110;
        sm->scan_step = 5;
        sm->state = STATE_SCAN_SERVO2;
        SM_PRINT("[SM] retry=1  servo0=45  SCAN_SERVO2 110->50\r\n");
        break;

    case 2:
        // X-scan: restore servo1 vertical, start diagonal pose
        smooth_move_servo1(sm->cur_servo1, 90, NORMAL_STEP_MS);
        sm->cur_servo1 = 90;
        smooth_move_servo0(sm->cur_servo0, 0, NORMAL_STEP_MS);
        sm->cur_servo0 = 0;
        smooth_move_servo2(sm->cur_servo2, 110, NORMAL_STEP_MS);
        sm->cur_servo2 = 110;
        sm->scan_angle = 0;
        sm->scan_step = 3;
        sm->scan_phase = 0;
        sm->state = STATE_SCAN_SERVO0;
        SM_PRINT("[SM] retry=2  X-scan phase 0  s0=0  s2=110\r\n");
        break;
    }
}

// ---- State name (debug) ----
const char *StateMachine_StateName(SystemState s) {
    switch (s) {
    case STATE_INIT:
        return "INIT";
    case STATE_SCAN_SERVO2:
        return "SCAN_SERVO2";
    case STATE_SCAN_SERVO0:
        return "SCAN_SERVO0";
    case STATE_CONFIRM:
        return "CONFIRM";
    case STATE_CALIBRATE:
        return "CALIBRATE";
    case STATE_CENTERED:
        return "CENTERED";
    case STATE_APPROACH:
        return "APPROACH";
    case STATE_CATCHING:
        return "CATCHING";
    case STATE_HALTING:
        return "HALTING";
    default:
        return "???";
    }
}

// ---- Init ----
void StateMachine_Init(StateMachine *sm) {
    sm->state = STATE_INIT;
    sm->scan_angle = 0;
    sm->scan_step = 0;
    sm->confirm_anchor_cx = 0.0f;
    sm->confirm_anchor_cy = 0.0f;
    sm->confirm_anchor_score = 0.0f;
    sm->confirm_anchor_width = 0.0f;
    sm->confirm_frame_count = 0;
    sm->confirm_success_count = 0;
    sm->calib_buf_idx = 0;
    sm->calib_sample_count = 0;
    sm->track_cx = 0.0f;
    sm->track_cy = 0.0f;
    sm->approach_step_count = 0;

    sm->lose_counter = 0;

    sm->scan_retry = 0;
    sm->scan_phase = 0;

    sm->cur_servo0 = 45; // initial pan angle
    sm->cur_servo1 = 90; // vertical (straight up)
    sm->cur_servo2 = 90; // forward-facing

    sm->pre_catch_servo0 = 45;
    sm->pre_catch_servo1 = 90;
    sm->pre_catch_servo2 = 90;

    // KP: each pixel ≈0.56° (90° FOV/160px); kp=0.15→10px offset → 0.15°
    sm->tof_distance_mm = 8192; // TOF no-data sentinel

    sm->kp_pan = KP_PAN;
    sm->kp_tilt = KP_TILT;
    sm->deadband_x = DEADBAND_X;
    sm->deadband_y = DEADBAND_Y;
}

// ---- Step — called once per frame ----
void StateMachine_Step(StateMachine *sm, const YoloDetectionResult *det) {
    const bool has_det = (det != NULL && det->count > 0);

    // Guard: max approach steps → reset arm and rescan
    if (sm->approach_step_count >= 7) {
        SM_PRINT("[SM] guard: max approach steps (%u) reached — reset s1->90  s2->90\r\n",
                 (unsigned)sm->approach_step_count);

        smooth_move_servo1(sm->cur_servo1, 90, NORMAL_STEP_MS);
        sm->cur_servo1 = 90;
        smooth_move_servo2(sm->cur_servo2, 90, NORMAL_STEP_MS);
        sm->cur_servo2 = 90;

        sm->approach_step_count = 0;
        sm->scan_retry = 0;
        prv_enter_scan_retry(sm);
        return;
    }

    // Guard: servo2 > 160° → roll back both s1 and s2 halfway to 90
    if (sm->cur_servo2 > 160) {
        int32_t s1_target = (int32_t)sm->cur_servo1 - ((int32_t)sm->cur_servo1 - 90) / 2;
        int32_t s2_target = (int32_t)sm->cur_servo2 - ((int32_t)sm->cur_servo2 - 90) / 2;

        SM_PRINT("[SM] guard: servo2=%d > 160  rollback s1=%d->%ld  s2=%d->%ld\r\n",
                 (int)sm->cur_servo2,
                 (int)sm->cur_servo1, (long)s1_target,
                 (int)sm->cur_servo2, (long)s2_target);

        smooth_move_servo1(sm->cur_servo1, (uint8_t)s1_target, NORMAL_STEP_MS);
        sm->cur_servo1 = (uint8_t)s1_target;

        smooth_move_servo2(sm->cur_servo2, (uint8_t)s2_target, NORMAL_STEP_MS);
        sm->cur_servo2 = (uint8_t)s2_target;

        sm->approach_step_count = 0;
        sm->scan_retry = 0;
        prv_enter_scan_retry(sm);
        return;
    }

    switch (sm->state) {
    // INIT: set initial scanning pose → SCAN_SERVO2
    case STATE_INIT:
        smooth_move_servo0(sm->cur_servo0, 45, NORMAL_STEP_MS);
        smooth_move_servo1(sm->cur_servo1, 90, NORMAL_STEP_MS);
        smooth_move_servo2(sm->cur_servo2, 110, NORMAL_STEP_MS);

        sm->cur_servo0 = 45;
        sm->cur_servo1 = 90;
        sm->cur_servo2 = 110;
        sm->scan_angle = 110;
        sm->scan_step = 1;
        sm->state = STATE_SCAN_SERVO2;
        SM_PRINT("[SM] INIT -> SCAN_SERVO2 (servo2=%ld deg)\r\n", (long)sm->scan_angle);
        break;

    // SCAN_SERVO2: vertical sweep 110°→50°, step=5°
    case STATE_SCAN_SERVO2:
        if (has_det) {
            // Save anchor and enter confirmation
            sm->confirm_anchor_cx = det->detections[0].x;
            sm->confirm_anchor_cy = det->detections[0].y;
            sm->confirm_anchor_score = det->detections[0].score;
            sm->confirm_anchor_width = det->detections[0].w;
            sm->confirm_frame_count = 0;
            sm->confirm_success_count = 0;
            sm->state = STATE_CONFIRM;
            SM_PRINT("[SM] SCAN_SERVO2 -> CONFIRM  anchor(%.1f,%.1f) score=%.2f w=%.1f\r\n",
                     (double)sm->confirm_anchor_cx, (double)sm->confirm_anchor_cy,
                     (double)sm->confirm_anchor_score,
                     (double)sm->confirm_anchor_width);
            break;
        }

        sm->scan_angle -= sm->scan_step;
        if (sm->scan_angle < 50) {
            // Vertical sweep done — advance retry level
            if (sm->scan_retry == 0) {
                sm->scan_retry = 1;
            } else {
                sm->scan_retry = 2;
            }
            prv_enter_scan_retry(sm);
            SM_PRINT("[SM] SCAN_SERVO2 -> retry=%u  (no target)\r\n",
                     (unsigned)sm->scan_retry);
        } else {
            smooth_move_servo2(sm->cur_servo2, (uint8_t)sm->scan_angle, SLOW_STEP_MS);
            sm->cur_servo2 = (uint8_t)sm->scan_angle;
        }
        break;

    // SCAN_SERVO0: X-shaped diagonal scan (3 phases)
    case STATE_SCAN_SERVO0:
        if (has_det) {
            // Save anchor and enter confirmation
            sm->confirm_anchor_cx = det->detections[0].x;
            sm->confirm_anchor_cy = det->detections[0].y;
            sm->confirm_anchor_score = det->detections[0].score;
            sm->confirm_anchor_width = det->detections[0].w;
            sm->confirm_frame_count = 0;
            sm->confirm_success_count = 0;
            sm->state = STATE_CONFIRM;
            SM_PRINT("[SM] SCAN_SERVO0 -> CONFIRM  anchor(%.1f,%.1f) score=%.2f w=%.1f\r\n",
                     (double)sm->confirm_anchor_cx, (double)sm->confirm_anchor_cy,
                     (double)sm->confirm_anchor_score,
                     (double)sm->confirm_anchor_width);
            break;
        }

        switch (sm->scan_phase) {
        // Phase 0: diagonal ↘  s0 0→90, s2 110→40
        case 0: {
            sm->scan_angle += sm->scan_step;
            if (sm->scan_angle >= 90) {
                sm->scan_angle = 90;
                smooth_move_servo0(sm->cur_servo0, 90, SLOW_STEP_MS);
                sm->cur_servo0 = 90;
                smooth_move_servo2(sm->cur_servo2, 40, SLOW_STEP_MS);
                sm->cur_servo2 = 40;
                sm->scan_phase = 1;
                SM_PRINT("[SM] X-scan phase 0 done -> phase 1 (fast return)\r\n");
                break;
            }
            // Proportional servo2: 110→40 as servo0 goes 0→90
            uint8_t s2_target = (uint8_t)(110.0f - sm->scan_angle * 70.0f / 90.0f);
            smooth_move_servo0(sm->cur_servo0, sm->scan_angle, SLOW_STEP_MS);
            sm->cur_servo0 = sm->scan_angle;
            smooth_move_servo2(sm->cur_servo2, s2_target, SLOW_STEP_MS);
            sm->cur_servo2 = s2_target;
            break;
        }

        // Phase 1: fast return ↗  s0=90, s2 40→110
        case 1: {
            smooth_move_servo0(sm->cur_servo0, 90, SLOW_STEP_MS);
            sm->cur_servo0 = 90;
            smooth_move_servo2(sm->cur_servo2, 110, SLOW_STEP_MS);
            sm->cur_servo2 = 110;
            sm->scan_angle = 90;
            sm->scan_phase = 2;
            SM_PRINT("[SM] X-scan phase 1 done -> phase 2 (reverse diagonal)\r\n");
            break;
        }

        // Phase 2: diagonal ↙  s0 90→0, s2 110→40
        case 2: {
            sm->scan_angle -= sm->scan_step;
            if (sm->scan_angle <= 0) {
                // X-scan complete — cycle back to centre + vertical
                sm->scan_retry = 1;
                prv_enter_scan_retry(sm);
                SM_PRINT("[SM] X-scan complete -> retry=1\r\n");
                break;
            }
            // Proportional servo2: 110→40 as servo0 goes 90→0
            uint8_t s2_target = (uint8_t)(110.0f - (90.0f - sm->scan_angle) * 70.0f / 90.0f);
            smooth_move_servo0(sm->cur_servo0, sm->scan_angle, SLOW_STEP_MS);
            sm->cur_servo0 = sm->scan_angle;
            smooth_move_servo2(sm->cur_servo2, s2_target, SLOW_STEP_MS);
            sm->cur_servo2 = s2_target;
            break;
        }
        }
        break;

    // CONFIRM: 30-frame window, need ≥2 matches within 30px of anchor
    case STATE_CONFIRM:
        sm->confirm_frame_count++;

        {
            bool matched = false;

            if (has_det) {
                for (uint32_t i = 0; i < det->count; i++) {
                    float dx = det->detections[i].x - sm->confirm_anchor_cx;
                    float dy = det->detections[i].y - sm->confirm_anchor_cy;
                    float d = sqrtf(dx * dx + dy * dy);

                    if (d <= 30.0f) {
                        sm->confirm_success_count++;
                        matched = true;
                        // Update anchor to tracked target position
                        sm->confirm_anchor_cx = det->detections[i].x;
                        sm->confirm_anchor_cy = det->detections[i].y;
                        break;
                    }
                }
            }

            SM_PRINT("[SM] CONFIRM %02u/30  anchor(%.1f,%.1f) "
                     "check=%u %s  success=%u\r\n",
                     (unsigned)sm->confirm_frame_count,
                     (double)sm->confirm_anchor_cx, (double)sm->confirm_anchor_cy,
                     has_det ? det->count : 0U,
                     matched ? "MATCH" : (has_det ? "MISMATCH" : "MISS"),
                     (unsigned)sm->confirm_success_count);
        }

        if (sm->confirm_success_count >= 2) {
            // Confirmed — start calibration
            sm->lose_counter = 0;
            sm->calib_buf_idx = 0;
            sm->calib_sample_count = 0;
            sm->scan_retry = 0;
            sm->track_cx = sm->confirm_anchor_cx;
            sm->track_cy = sm->confirm_anchor_cy;
            sm->state = STATE_CALIBRATE;
            SM_PRINT("[SM] CONFIRM -> CALIBRATE  %u/30\r\n",
                     (unsigned)sm->confirm_success_count);
        } else if (sm->confirm_frame_count >= 30) {
            // Failed — reset to retry=0 (vertical sweep from current pos)
            sm->lose_counter = 0;
            sm->scan_retry = 0;
            prv_enter_scan_retry(sm);
            SM_PRINT("[SM] CONFIRM -> retry=%u  fail %u/30\r\n",
                     (unsigned)sm->scan_retry,
                     (unsigned)sm->confirm_success_count);
        }
        break;

    // CALIBRATE: 2-frame sliding-window average centering via P-controller
    case STATE_CALIBRATE:
        if (has_det) {
            sm->lose_counter = 0;

            // Use widest detection (already sorted by width in YoloApi)
            float cx = det->detections[0].x;
            float cy = det->detections[0].y;
            sm->track_cx = cx;
            sm->track_cy = cy;

            // Store in sliding-window circular buffer
            sm->calib_cx_buf[sm->calib_buf_idx] = cx;
            sm->calib_cy_buf[sm->calib_buf_idx] = cy;
            sm->calib_buf_idx = (sm->calib_buf_idx + 1) % CALIB_WIN_SIZE;
            sm->calib_sample_count++;

            // First sample: collect only, no movement
            if (sm->calib_sample_count < CALIB_WIN_SIZE) {
                SM_PRINT("[SM] CALIBRATE: collect #%u  (%.1f,%.1f)\r\n",
                         (unsigned)sm->calib_sample_count, (double)cx, (double)cy);
                break;
            }

            // Average the values in the sliding window
            float sum_cx = 0.0f, sum_cy = 0.0f;
            for (uint32_t i = 0; i < CALIB_WIN_SIZE; i++) {
                sum_cx += sm->calib_cx_buf[i];
                sum_cy += sm->calib_cy_buf[i];
            }
            const float cx_avg = sum_cx / (float)CALIB_WIN_SIZE;
            const float cy_avg = sum_cy / (float)CALIB_WIN_SIZE;
            const float off_x = cx_avg - CX_TARGET;
            const float off_y = cy_avg - CY_TARGET;

            // Check deadband — already centred?
            if (fabsf(off_x) < sm->deadband_x && fabsf(off_y) < sm->deadband_y) {
                sm->state = STATE_CENTERED;
                SM_PRINT("[SM] CALIBRATE -> CENTERED  avg(%.1f,%.1f)\r\n",
                         (double)cx_avg, (double)cy_avg);
                break;
            }

            // P-controller deltas (half-step)
            int32_t delta_pan = (int32_t)(off_x * 0.5f * sm->kp_pan);
            int32_t delta_tilt = (int32_t)(-off_y * 0.5f * sm->kp_tilt);

            SM_PRINT("[SM] CALIBRATE: avg(%.1f,%.1f) off(%.1f,%.1f) dp=%ld dt=%ld\r\n",
                     (double)cx_avg, (double)cy_avg, (double)off_x, (double)off_y,
                     (long)delta_pan, (long)delta_tilt);

            // Apply servo0 (pan)
            int32_t from0 = sm->cur_servo0;
            int32_t new_servo0 = clamp_i32(from0 + delta_pan, -135, 135);
            int32_t new_s2 = (int32_t)sm->cur_servo2 + delta_tilt;
            uint8_t new_servo2 = clamp_u8(new_s2, 0, 180);

            SM_PRINT("[SM] CALIBRATE: s0 %ld->%ld  s2 %d->%d\r\n",
                     (long)from0, (long)new_servo0,
                     (int)sm->cur_servo2, (int)new_servo2);

            smooth_move_servo0(from0, new_servo0, NORMAL_STEP_MS);
            sm->cur_servo0 = new_servo0;

            // Apply servo2 (tilt)
            smooth_move_servo2(sm->cur_servo2, new_servo2, NORMAL_STEP_MS);
            sm->cur_servo2 = new_servo2;

            delay_ms(300); // settle after movement

            // Reset sliding window for fresh frames after each move
            sm->calib_buf_idx = 0;
            sm->calib_sample_count = 0;

            // Servo1 remains fixed during centering
        } else {
            sm->lose_counter++;
            if (sm->lose_counter >= MAX_LOSE_FRAMES) {
                sm->lose_counter = 0;
                prv_enter_scan_retry(sm);
                SM_PRINT("[SM] CALIBRATE -> retry=%u  (lost target)\r\n",
                         (unsigned)sm->scan_retry);
            }
        }
        break;

    // CENTERED: target centred, monitor drift/loss; decide APPROACH or CATCHING
    case STATE_CENTERED:
        if (has_det) {
            sm->lose_counter = 0;

            // Use widest detection (already sorted by width in YoloApi)
            float cx = det->detections[0].x;
            float cy = det->detections[0].y;
            float bb_width = det->detections[0].w;
            sm->track_cx = cx;
            sm->track_cy = cy;

            // Accumulate into 3-frame sliding window for drift detection
            sm->calib_cx_buf[sm->calib_buf_idx] = cx;
            sm->calib_cy_buf[sm->calib_buf_idx] = cy;
            sm->calib_buf_idx = (sm->calib_buf_idx + 1) % CALIB_WIN_SIZE;
            if (sm->calib_sample_count < CALIB_WIN_SIZE)
                sm->calib_sample_count++;

            // Check drift: average window frames before deciding
            if (sm->calib_sample_count >= CALIB_WIN_SIZE) {
                float sum_cx = 0.0f, sum_cy = 0.0f;
                for (uint32_t i = 0; i < CALIB_WIN_SIZE; i++) {
                    sum_cx += sm->calib_cx_buf[i];
                    sum_cy += sm->calib_cy_buf[i];
                }
                const float cx_avg = sum_cx / (float)CALIB_WIN_SIZE;
                const float cy_avg = sum_cy / (float)CALIB_WIN_SIZE;
                const float off_x = cx_avg - CX_TARGET;
                const float off_y = cy_avg - CY_TARGET;

                if (fabsf(off_x) > sm->deadband_x || fabsf(off_y) > sm->deadband_y) {
                    sm->calib_buf_idx = 0;
                    sm->calib_sample_count = 0;
                    sm->state = STATE_CALIBRATE;
                    SM_PRINT("[SM] CENTERED -> CALIBRATE  (drift, avg=%.1f,%.1f)\r\n",
                             (double)cx_avg, (double)cy_avg);
                    break;
                }
            }

            // Check blueberry width to decide APPROACH vs CATCHING
            if (bb_width < CATCH_WIDTH_THRESH) {
                // Too far — start approaching
                sm->approach_step_count = 0;
                sm->state = STATE_APPROACH;
                SM_PRINT("[SM] CENTERED -> APPROACH  width=%.1f\r\n",
                         (double)bb_width);
            } else {
                // Width >= CATCH_WIDTH_THRESH — close enough, start pickup
                sm->pre_catch_servo0 = sm->cur_servo0;
                sm->pre_catch_servo1 = sm->cur_servo1;
                sm->pre_catch_servo2 = sm->cur_servo2;
                sm->state = STATE_CATCHING;
                SM_PRINT("[SM] CENTERED -> CATCHING  width=%.1f  "
                         "pre(servo0=%ld, servo1=%u, servo2=%u)\r\n",
                         (double)bb_width,
                         (long)sm->pre_catch_servo0,
                         (unsigned)sm->pre_catch_servo1,
                         (unsigned)sm->pre_catch_servo2);
            }
        } else {
            sm->lose_counter++;
            if (sm->lose_counter >= MAX_LOSE_FRAMES) {
                sm->lose_counter = 0;
                prv_enter_scan_retry(sm);
                SM_PRINT("[SM] CENTERED -> retry=%u  (lost target)\r\n",
                         (unsigned)sm->scan_retry);
            }
        }
        break;

    // APPROACH: s1 -3°, s2 +4° per step, then → CALIBRATE to re-centre
    case STATE_APPROACH:
        {
            int32_t step_deg = 3;

            int32_t new_s1 = (int32_t)sm->cur_servo1 - step_deg;
            int32_t new_s2 = (int32_t)sm->cur_servo2 + step_deg + 1; // +2° extra to compensate for arm tilt
            uint8_t clamped_s1 = clamp_u8(new_s1, 0, 180);
            uint8_t clamped_s2 = clamp_u8(new_s2, 0, 180);

            SM_PRINT("[SM] APPROACH: s1 %d->%d  s2 %d->%d  step=%u  deg=%d\r\n",
                     (int)sm->cur_servo1, (int)clamped_s1,
                     (int)sm->cur_servo2, (int)clamped_s2,
                     (unsigned)sm->approach_step_count + 1,
                     step_deg);

            smooth_move_servo1(sm->cur_servo1, clamped_s1, NORMAL_STEP_MS);
            sm->cur_servo1 = clamped_s1;

            smooth_move_servo2(sm->cur_servo2, clamped_s2, NORMAL_STEP_MS);
            sm->cur_servo2 = clamped_s2;

            delay_ms(150); // settle

            sm->approach_step_count++;

            // Reset sliding window for fresh frames after each move
            sm->calib_buf_idx = 0;
            sm->calib_sample_count = 0;
            sm->state = STATE_CALIBRATE;
            SM_PRINT("[SM] APPROACH -> CALIBRATE  (re-centre after step %u)\r\n",
                     (unsigned)sm->approach_step_count);
        }
        break;

    // CATCHING: pickup sequence (微张→tilt up→close gripper→lift→rotate→drop→open)
    case STATE_CATCHING: {
        SM_PRINT("[SM] CATCHING: starting pickup sequence\r\n");

        servo_catch_set_pulse(1300); // slightly open gripper
        delay_ms(100);

        // Tilt servo2 up before pickup to clear the target
        {
            uint8_t new_s2 = clamp_u8((int32_t)sm->cur_servo2 + 10, 0, 180);
            servo2_set_angle(new_s2);
            sm->cur_servo2 = new_s2;
            delay_ms(100);
        }

        servo_catch_close_slow(600);
        delay_ms(200); // wait for gripper to close

        // servo1 → 100° (return to vertical)
        smooth_move_servo1(sm->cur_servo1, 100, NORMAL_STEP_MS);
        sm->cur_servo1 = 100;
        delay_ms(100);

        // servo2 → +20° (tilt up to clear)
        {
            uint8_t new_s2 = clamp_u8((int32_t)sm->cur_servo2 + 20, 0, 180);
            smooth_move_servo2(sm->cur_servo2, new_s2, NORMAL_STEP_MS);
            sm->cur_servo2 = new_s2;
        }
        delay_ms(100);

        // servo0 → -45° (rotate base)
        smooth_move_servo0(sm->cur_servo0, -45, NORMAL_STEP_MS);
        sm->cur_servo0 = -45;
        delay_ms(100);

        // servo2 → 45° (tilt down to drop position)
        smooth_move_servo2(sm->cur_servo2, 45, NORMAL_STEP_MS);
        sm->cur_servo2 = 45;
        delay_ms(100);

        // servo1 → 90° (return to vertical)
        smooth_move_servo1(sm->cur_servo1, 90, NORMAL_STEP_MS);
        sm->cur_servo1 = 90;
        delay_ms(100);

        // servo0 → 30° (rotate to drop position)
        smooth_move_servo0(sm->cur_servo0, 30, NORMAL_STEP_MS);
        sm->cur_servo0 = 30;
        delay_ms(200);

        // Open gripper (松开夹爪放下目标)
        servo_catch_open();
        delay_ms(200); // wait for gripper to open

        SM_PRINT("[SM] CATCHING: pickup complete  -> HALTING\r\n");
        sm->state = STATE_HALTING;
        break;
    }

    // HALTING: restore arm to scanning pose based on pre-catch snapshot → SCAN_SERVO2
    case STATE_HALTING: {
        // s1 halfway back toward 90°, s2 tilt down by same delta
        int32_t s1_delta = ((int32_t)90 - sm->pre_catch_servo1) * 2 / 3;
        int32_t s1_target = (int32_t)sm->pre_catch_servo1 + s1_delta;
        int32_t s2_raw = (int32_t)sm->pre_catch_servo2 - s1_delta; // s2 decreases by same amount
        uint8_t s2_target = clamp_u8(s2_raw, 0, 180);

        SM_PRINT("[SM] HALTING: pre(servo0=%ld, servo1=%u, servo2=%u)  "
                 "s1_target=%ld  s2_target=%d\r\n",
                 (long)sm->pre_catch_servo0,
                 (unsigned)sm->pre_catch_servo1,
                 (unsigned)sm->pre_catch_servo2,
                 (long)s1_target, (int)s2_target);

        // Restore servo0 to pre-catch angle
        smooth_move_servo0(sm->cur_servo0, sm->pre_catch_servo0, NORMAL_STEP_MS);
        sm->cur_servo0 = sm->pre_catch_servo0;

        // Restore arm to scanning pose
        smooth_move_servo1(sm->cur_servo1, (uint8_t)s1_target, NORMAL_STEP_MS);
        sm->cur_servo1 = (uint8_t)s1_target;

        smooth_move_servo2(sm->cur_servo2, s2_target, NORMAL_STEP_MS);
        sm->cur_servo2 = s2_target;

        delay_ms(500);

        SM_PRINT("[SM] HALTING: scanning pose reached -> SCAN_SERVO2\r\n");

        // Resume scanning from current servo2 position
        sm->scan_angle = (int32_t)sm->cur_servo2;
        sm->scan_step = 1;
        sm->scan_retry = 0;
        sm->state = STATE_SCAN_SERVO2;
        break;
    }
    } // switch
}
