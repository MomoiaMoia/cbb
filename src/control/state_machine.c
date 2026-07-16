/**
 * @file    state_machine.c
 * @brief   Blueberry-picking state machine implementation
 *
 * Physical layout (confirmed by user):
 *   Servo0  — base rotation (270° pan)        -135 … +135, neutral 0°
 *   Servo1  — shoulder joint (180°)             0 … 180, neutral 90°
 *             0° / 180° = horizontal,  90° = vertical
 *   Servo2  — elbow / camera tilt (180°)        0 … 180, neutral 90°
 *             Mounted on servo1; camera on servo2.
 *             When servo1 = 90° (vertical),
 *               0° = camera down,  90° = forward,  180° = up
 *
 * Centering:
 *   cx offset  →  servo0 (pan)     delta_pan  = off_x * kp_pan
 *   cy offset  →  servo2 (tilt)    delta_tilt = -off_y * kp_tilt
 *
 * NOTE: cx, cy are in pixel coords within the 160x160 image.
 *       Targets defined at top of file (CX_TARGET, CY_TARGET).
 **********************************************************************************************************************/
#include "state_machine.h"

#include "common_utils.h"
#include "hal_data.h"
#include "servos/servo0.h"
#include "servos/servo1.h"
#include "servos/servo2.h"
#include "servos/servo_catch.h"

#include <math.h>

/* ------------------------------------------------------------------ */
/*  Debug output: route to SEGGER RTT (1) or UART printf (0)          */
/* ------------------------------------------------------------------ */
#define SM_DBG_USE_RTT 0

#if SM_DBG_USE_RTT
#define SM_PRINT(fmt, ...) APP_PRINT(fmt, ##__VA_ARGS__)
#else
#define SM_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#endif

/* ------------------------------------------------------------------ */
/*  Calibration target (pixel coords, 0-160)                          */
/* ------------------------------------------------------------------ */
#define CX_TARGET 80.0f
#define CY_TARGET 95.0f

/* ------------------------------------------------------------------ */
/*  P-controller gains & deadband                                      */
/* ------------------------------------------------------------------ */
#define KP_PAN 0.20f
#define KP_TILT 0.20f
#define DEADBAND_X 16.0f
#define DEADBAND_Y 16.0f
#define MIN_DET_W 10.0f

/* ------------------------------------------------------------------ */
/*  Local helpers                                                      */
/* ------------------------------------------------------------------ */

/** Clamp int32_t to [lo, hi]. */
static int32_t clamp_i32(int32_t val, int32_t lo, int32_t hi) {
    if (val < lo)
        return lo;
    if (val > hi)
        return hi;
    return val;
}

/** Clamp int32_t to [lo, hi] and return as uint8_t. */
static uint8_t clamp_u8(int32_t val, int32_t lo, int32_t hi) {
    if (val < lo)
        return (uint8_t)lo;
    if (val > hi)
        return (uint8_t)hi;
    return (uint8_t)val;
}

/**
 * Blocking delay using Renesas FSP software delay.
 */
static void delay_ms(uint32_t ms) {
    if (ms == 0)
        return;
    R_BSP_SoftwareDelay(ms, BSP_DELAY_UNITS_MILLISECONDS);
}

/* ------------------------------------------------------------------ */
/*  Smooth movement helpers — gradual 1°/step motion                   */
/* ------------------------------------------------------------------ */

/** Step delay (ms) per degree — speed levels. */
#define SLOW_STEP_MS 30
#define NORMAL_STEP_MS 15
#define FAST_STEP_MS 5

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

/* ------------------------------------------------------------------ */
/*  Scan retry ladder — called when target is lost                     */
/* ------------------------------------------------------------------ */

/**
 * Enter the appropriate scanning state based on sm->scan_retry.
 *
 * Retry levels:
 *   0 — keep current servo0, sweep servo2 40→110 (first retry)
 *   1 — reset servo0=45, sweep servo2 40→110 (centre + vertical)
 *   2 — X-scan: s0 -45→135 + s2 110→40 diagonal, then reverse
 *   After level 2 → back to level 1 (cycle)
 */
static void prv_enter_scan_retry(StateMachine *sm) {
    switch (sm->scan_retry) {
    case 0:
        /* Keep current servo0, start vertical sweep (bottom→up) */
        smooth_move_servo2(sm->cur_servo2, 40, FAST_STEP_MS);
        sm->cur_servo2 = 40;
        sm->scan_angle = 40;
        sm->scan_step = 5;
        sm->state = STATE_SCAN_SERVO2;
        SM_PRINT("[SM] retry=0  servo0=%ld  SCAN_SERVO2 40->110\r\n", (long)sm->cur_servo0);
        break;

    case 1:
        /* Reset to centre, restore servo1 vertical, vertical sweep (bottom→up) */
        smooth_move_servo0(sm->cur_servo0, 45, FAST_STEP_MS);
        sm->cur_servo0 = 45;
        smooth_move_servo1(sm->cur_servo1, 90, FAST_STEP_MS);
        sm->cur_servo1 = 90;
        smooth_move_servo2(sm->cur_servo2, 40, FAST_STEP_MS);
        sm->cur_servo2 = 40;
        sm->scan_angle = 40;
        sm->scan_step = 5;
        sm->state = STATE_SCAN_SERVO2;
        SM_PRINT("[SM] retry=1  servo0=45  SCAN_SERVO2 40->110\r\n");
        break;

    case 2:
        /* X-scan: restore servo1 vertical, start diagonal pose */
        smooth_move_servo1(sm->cur_servo1, 90, FAST_STEP_MS);
        sm->cur_servo1 = 90;
        smooth_move_servo0(sm->cur_servo0, 0, FAST_STEP_MS);
        sm->cur_servo0 = 0;
        smooth_move_servo2(sm->cur_servo2, 110, FAST_STEP_MS);
        sm->cur_servo2 = 110;
        sm->scan_angle = 0;
        sm->scan_step = 3;
        sm->scan_phase = 0;
        sm->state = STATE_SCAN_SERVO0;
        SM_PRINT("[SM] retry=2  X-scan phase 0  s0=0  s2=110\r\n");
        break;
    }
}

/* ------------------------------------------------------------------ */
/*  State name (debug)                                                 */
/* ------------------------------------------------------------------ */
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

/* ------------------------------------------------------------------ */
/*  Init                                                               */
/* ------------------------------------------------------------------ */
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
    sm->max_lose_frames = 60;
    sm->scan_retry = 0;
    sm->scan_phase = 0;

    sm->cur_servo0 = 45; // initial pan angle
    sm->cur_servo1 = 90; // vertical (straight up)
    sm->cur_servo2 = 90; // forward-facing

    sm->pre_catch_servo0 = 45;
    sm->pre_catch_servo1 = 90;
    sm->pre_catch_servo2 = 90;

    /* KP values: pixel coords (0-160), centre at 80.
     * Each pixel ≈ 0.5625° (90° FOV / 160px)
     * kp=0.15 → 10px偏差移0.15°
     */
    /* TOF — default to no-data sentinel */
    sm->tof_distance_mm = 8192;

    sm->kp_pan = KP_PAN;
    sm->kp_tilt = KP_TILT;
    sm->deadband_x = DEADBAND_X;
    sm->deadband_y = DEADBAND_Y;
}

/* ------------------------------------------------------------------ */
/*  Step — called once per frame                                       */
/* ------------------------------------------------------------------ */
void StateMachine_Step(StateMachine *sm, const YoloDetectionResult *det) {
    const bool has_det = (det != NULL && det->count > 0);

    /* Guard: if approach steps exceed limit, restore arm and give up */
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

    /* Guard: if servo2 exceeds 160°, roll back both s1 and s2 halfway to 90 */
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
    /* ================================================================
     *  INIT — set initial scanning pose
     * ================================================================ */
    case STATE_INIT:
        smooth_move_servo0(sm->cur_servo0, 45, NORMAL_STEP_MS);
        smooth_move_servo1(sm->cur_servo1, 90, NORMAL_STEP_MS);
        smooth_move_servo2(sm->cur_servo2, 110, NORMAL_STEP_MS);

        sm->cur_servo0 = 45;
        sm->cur_servo1 = 90;
        sm->cur_servo2 = 40;
        sm->scan_angle = 40;
        sm->scan_step = 1;
        sm->state = STATE_SCAN_SERVO2;
        SM_PRINT("[SM] INIT -> SCAN_SERVO2 (servo2=%ld deg)\r\n", (long)sm->scan_angle);
        break;

    /* ================================================================
     *  SCAN_SERVO2 — vertical sweep  40° → 110°
     * ================================================================ */
    case STATE_SCAN_SERVO2:
        if (has_det && det->detections[0].w >= MIN_DET_W) {
            /* Save anchor and enter confirmation */
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

        sm->scan_angle += sm->scan_step;
        if (sm->scan_angle > 110) {
            /* Vertical sweep done — advance retry level */
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

    /* ================================================================
     *  SCAN_SERVO0 — X-shaped diagonal scan
     *
     *  Phase 0: s0 0→90,  s2 110→40  (diagonal down-right  ↘)
     *  Phase 1: s0 stays,  s2 40→110   (fast return up        ↗)
     *  Phase 2: s0 90→0,   s2 110→40   (diagonal down-left    ↙)
     *  After phase 2 → cycle back to retry=1
     * ================================================================ */
    case STATE_SCAN_SERVO0:
        if (has_det && det->detections[0].w >= MIN_DET_W) {
            /* Save anchor and enter confirmation */
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
        /* ---- Phase 0: diagonal ↘  s0 0→90, s2 110→40 ---- */
        case 0: {
            sm->scan_angle += sm->scan_step; /* servo0 increases */
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
            /* Proportional servo2: 110 → 40 as servo0 goes 0 → 90 */
            uint8_t s2_target = (uint8_t)(110.0f - sm->scan_angle * 70.0f / 90.0f);
            smooth_move_servo0(sm->cur_servo0, sm->scan_angle, SLOW_STEP_MS);
            sm->cur_servo0 = sm->scan_angle;
            smooth_move_servo2(sm->cur_servo2, s2_target, SLOW_STEP_MS);
            sm->cur_servo2 = s2_target;
            break;
        }

        /* ---- Phase 1: fast return ↗  s0 stays at 90, s2 40→110 ---- */
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

        /* ---- Phase 2: diagonal ↙  s0 90→0, s2 110→40 ---- */
        case 2: {
            sm->scan_angle -= sm->scan_step; /* servo0 decreases */
            if (sm->scan_angle <= 0) {
                /* X-scan complete — cycle back to centre + vertical */
                sm->scan_retry = 1;
                prv_enter_scan_retry(sm);
                SM_PRINT("[SM] X-scan complete -> retry=1\r\n");
                break;
            }
            /* Proportional servo2: 110 → 40 as servo0 goes 90 → 0 */
            uint8_t s2_target = (uint8_t)(110.0f - (90.0f - sm->scan_angle) * 70.0f / 90.0f);
            smooth_move_servo0(sm->cur_servo0, sm->scan_angle, SLOW_STEP_MS);
            sm->cur_servo0 = sm->scan_angle;
            smooth_move_servo2(sm->cur_servo2, s2_target, SLOW_STEP_MS);
            sm->cur_servo2 = s2_target;
            break;
        }
        }
        break;

    /* ================================================================
     *  CONFIRM — 30-frame window, need ≥2 matches within 30 px of anchor
     *
     *  Checks up to 3 detections per frame (if multiple targets exist).
     *  Any detection within 30 px of anchor counts as a match.
     * ================================================================ */
    case STATE_CONFIRM:
        sm->confirm_frame_count++;

        {
            bool matched = false;

            if (has_det) {
                uint32_t check_cnt = det->count;
                if (check_cnt > 3)
                    check_cnt = 3;

                for (uint32_t i = 0; i < check_cnt; i++) {
                    if (det->detections[i].w < MIN_DET_W)
                        continue;

                    float dx = det->detections[i].x - sm->confirm_anchor_cx;
                    float dy = det->detections[i].y - sm->confirm_anchor_cy;
                    float d = sqrtf(dx * dx + dy * dy);

                    if (d <= 30.0f) {
                        sm->confirm_success_count++;
                        matched = true;
                        break;
                    }

                    /* Switch to a wider target — move centre toward it */
                    if (det->detections[i].w > sm->confirm_anchor_width + 5.0f) {
                        float off_x = det->detections[i].x - sm->confirm_anchor_cx;
                        float off_y = det->detections[i].y - sm->confirm_anchor_cy;

                        int32_t delta_pan = (int32_t)(off_x * 0.5f * sm->kp_pan);
                        int32_t delta_tilt = (int32_t)(-off_y * 0.5f * sm->kp_tilt);

                        int32_t new_s0 = clamp_i32(sm->cur_servo0 + delta_pan, -135, 135);
                        uint8_t new_s2 = clamp_u8((int32_t)sm->cur_servo2 + delta_tilt, 0, 180);

                        smooth_move_servo0(sm->cur_servo0, new_s0, NORMAL_STEP_MS);
                        sm->cur_servo0 = new_s0;
                        smooth_move_servo2(sm->cur_servo2, new_s2, NORMAL_STEP_MS);
                        sm->cur_servo2 = new_s2;

                        sm->confirm_anchor_cx = det->detections[i].x;
                        sm->confirm_anchor_cy = det->detections[i].y;
                        sm->confirm_anchor_score = det->detections[i].score;
                        sm->confirm_anchor_width = det->detections[i].w;
                        sm->confirm_frame_count = 0;
                        sm->confirm_success_count = 0;
                        matched = false;
                        SM_PRINT("[SM] CONFIRM  switch to wider target -> (%.1f,%.1f) "
                                 "w=%.1f (>%.1f+5)  move s0=%ld s2=%d\r\n",
                                 (double)sm->confirm_anchor_cx, (double)sm->confirm_anchor_cy,
                                 (double)det->detections[i].w,
                                 (double)sm->confirm_anchor_width,
                                 (long)new_s0, (int)new_s2);
                        break;
                    }
                }
            }

            SM_PRINT("[SM] CONFIRM %02u/30  anchor(%.1f,%.1f) "
                     "check=%u %s  success=%u\r\n",
                     (unsigned)sm->confirm_frame_count,
                     (double)sm->confirm_anchor_cx, (double)sm->confirm_anchor_cy,
                     has_det ? (det->count > 3 ? 3U : det->count) : 0U,
                     matched ? "MATCH" : (has_det ? "MISMATCH" : "MISS"),
                     (unsigned)sm->confirm_success_count);
        }

        if (sm->confirm_success_count >= 2) {
            /* Confirmed — start calibration, reset sliding window */
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
            /* Failed — reset to retry=0 (vertical sweep from current pos) */
            sm->lose_counter = 0;
            sm->scan_retry = 0;
            prv_enter_scan_retry(sm);
            SM_PRINT("[SM] CONFIRM -> retry=%u  fail %u/30\r\n",
                     (unsigned)sm->scan_retry,
                     (unsigned)sm->confirm_success_count);
        }
        break;

    /* ================================================================
     *  CALIBRATE — 3-frame sliding-window average centering
     *
     *  First 2 samples: collect only, no movement.
     *  From sample 3 onward: average the 3 values in the sliding window
     *  and execute one smooth move. Reset buffer after each move.
     *
     *  Servo0 ← cx offset (pan)
     *  Servo2 ← cy offset (tilt)
     *  Servo1 fixed at current angle
     * ================================================================ */
    case STATE_CALIBRATE:
        if (has_det) {
            sm->lose_counter = 0;

            /* Find detection closest to the target we're tracking (skip narrow) */
            float cx = 0.0f, cy = 0.0f, best_dist = 0.0f;
            bool found = false;
            for (uint32_t i = 0; i < det->count; i++) {
                if (det->detections[i].w < MIN_DET_W)
                    continue;
                float d = fabsf(det->detections[i].x - sm->track_cx) + fabsf(det->detections[i].y - sm->track_cy);
                if (!found || d < best_dist) {
                    best_dist = d;
                    cx = det->detections[i].x;
                    cy = det->detections[i].y;
                    found = true;
                }
            }
            if (!found) {
                sm->lose_counter++;
                if (sm->lose_counter >= sm->max_lose_frames) {
                    sm->lose_counter = 0;
                    prv_enter_scan_retry(sm);
                }
                break;
            }
            sm->track_cx = cx;
            sm->track_cy = cy;

            /* Store in sliding-window circular buffer */
            sm->calib_cx_buf[sm->calib_buf_idx] = cx;
            sm->calib_cy_buf[sm->calib_buf_idx] = cy;
            sm->calib_buf_idx = (sm->calib_buf_idx + 1) % 3;
            sm->calib_sample_count++;

            /* First 2 samples: collect only, no movement */
            if (sm->calib_sample_count < 3) {
                SM_PRINT("[SM] CALIBRATE: collect #%u  (%.1f,%.1f)\r\n",
                         (unsigned)sm->calib_sample_count, (double)cx, (double)cy);
                break;
            }

            /* Average the 3 values in the sliding window */
            float sum_cx = 0.0f, sum_cy = 0.0f;
            for (uint32_t i = 0; i < 3; i++) {
                sum_cx += sm->calib_cx_buf[i];
                sum_cy += sm->calib_cy_buf[i];
            }
            const float cx_avg = sum_cx / 3.0f;
            const float cy_avg = sum_cy / 3.0f;
            const float off_x = cx_avg - CX_TARGET;
            const float off_y = cy_avg - CY_TARGET;

            /* Check deadband — already centred? */
            if (fabsf(off_x) < sm->deadband_x && fabsf(off_y) < sm->deadband_y) {
                sm->state = STATE_CENTERED;
                SM_PRINT("[SM] CALIBRATE -> CENTERED  avg(%.1f,%.1f)\r\n",
                         (double)cx_avg, (double)cy_avg);
                break;
            }

            /* --- Calculate deltas from averaged offset (P-controller, half-step) --- */
            int32_t delta_pan = (int32_t)(off_x * 0.5f * sm->kp_pan);
            int32_t delta_tilt = (int32_t)(-off_y * 0.5f * sm->kp_tilt);

            SM_PRINT("[SM] CALIBRATE: avg(%.1f,%.1f) off(%.1f,%.1f) dp=%ld dt=%ld\r\n",
                     (double)cx_avg, (double)cy_avg, (double)off_x, (double)off_y,
                     (long)delta_pan, (long)delta_tilt);

            /* --- Apply Servo0 (pan) --- */
            int32_t from0 = sm->cur_servo0;
            int32_t new_servo0 = clamp_i32(from0 + delta_pan, -135, 135);
            int32_t new_s2 = (int32_t)sm->cur_servo2 + delta_tilt;
            uint8_t new_servo2 = clamp_u8(new_s2, 0, 180);

            SM_PRINT("[SM] CALIBRATE: s0 %ld->%ld  s2 %d->%d\r\n",
                     (long)from0, (long)new_servo0,
                     (int)sm->cur_servo2, (int)new_servo2);

            smooth_move_servo0(from0, new_servo0, NORMAL_STEP_MS);
            sm->cur_servo0 = new_servo0;

            /* --- Apply Servo2 (tilt) --- */
            smooth_move_servo2(sm->cur_servo2, new_servo2, NORMAL_STEP_MS);
            sm->cur_servo2 = new_servo2;

            delay_ms(300); // settle after movement

            /* Reset sliding window — fresh 5 frames after each move */
            sm->calib_buf_idx = 0;
            sm->calib_sample_count = 0;

            /* Servo1 remains fixed during centering */
        } else {
            sm->lose_counter++;
            if (sm->lose_counter >= sm->max_lose_frames) {
                sm->lose_counter = 0;
                prv_enter_scan_retry(sm);
                SM_PRINT("[SM] CALIBRATE -> retry=%u  (lost target)\r\n",
                         (unsigned)sm->scan_retry);
            }
        }
        break;

    /* ================================================================
     *  CENTERED — target centred, monitor drift / loss
     *
     *  Drift detection uses 3-frame sliding window average (same buffer
     *  as CALIBRATE, reset on entry) to avoid false positives from
     *  single-frame jitter.
     * ================================================================ */
    case STATE_CENTERED:
        if (has_det) {
            sm->lose_counter = 0;

            /* Find detection closest to the target we're tracking (skip narrow) */
            float cx = 0.0f, cy = 0.0f, best_dist = 0.0f;
            bool found = false;
            for (uint32_t i = 0; i < det->count; i++) {
                if (det->detections[i].w < MIN_DET_W)
                    continue;
                float d = fabsf(det->detections[i].x - sm->track_cx) + fabsf(det->detections[i].y - sm->track_cy);
                if (!found || d < best_dist) {
                    best_dist = d;
                    cx = det->detections[i].x;
                    cy = det->detections[i].y;
                    found = true;
                }
            }
            if (!found) {
                sm->lose_counter++;
                if (sm->lose_counter >= sm->max_lose_frames) {
                    sm->lose_counter = 0;
                    prv_enter_scan_retry(sm);
                }
                break;
            }
            sm->track_cx = cx;
            sm->track_cy = cy;

            /* Accumulate into 3-frame sliding window */
            sm->calib_cx_buf[sm->calib_buf_idx] = cx;
            sm->calib_cy_buf[sm->calib_buf_idx] = cy;
            sm->calib_buf_idx = (sm->calib_buf_idx + 1) % 3;
            if (sm->calib_sample_count < 3)
                sm->calib_sample_count++;

            /* Check drift: average 3 frames before deciding */
            if (sm->calib_sample_count >= 3) {
                float sum_cx = 0.0f, sum_cy = 0.0f;
                for (uint32_t i = 0; i < 3; i++) {
                    sum_cx += sm->calib_cx_buf[i];
                    sum_cy += sm->calib_cy_buf[i];
                }
                const float cx_avg = sum_cx / 3.0f;
                const float cy_avg = sum_cy / 3.0f;
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

            /* Check blueberry width & TOF distance to decide approach or pickup */
            float bb_width = det->detections[0].w;
            if (bb_width < 40.0f) {
                /* Small — too far, start approaching */
                sm->approach_step_count = 0;
                sm->state = STATE_APPROACH;
                SM_PRINT("[SM] CENTERED -> APPROACH  width=%.1f\r\n",
                         (double)bb_width);
            } else if (sm->tof_distance_mm <= 50 || bb_width > 40.0f) {
                /* Width >= 40 AND TOF <= 50mm — close enough, start pickup */
                sm->pre_catch_servo0 = sm->cur_servo0;
                sm->pre_catch_servo1 = sm->cur_servo1;
                sm->pre_catch_servo2 = sm->cur_servo2;

                /* Already close enough — start pickup */
                sm->state = STATE_CATCHING;
                SM_PRINT("[SM] CENTERED -> CATCHING  width=%.1f  tof=%u mm  "
                         "pre(servo0=%ld, servo1=%u, servo2=%u)\r\n",
                         (double)bb_width,
                         (unsigned)sm->tof_distance_mm,
                         (long)sm->pre_catch_servo0,
                         (unsigned)sm->pre_catch_servo1,
                         (unsigned)sm->pre_catch_servo2);
            } else {
                /* Width >= 40 but TOF still > 50mm — wait, keep centred */
                SM_PRINT("[SM] CENTERED  width=%.1f  tof=%u mm  (waiting for TOF <= 50)\r\n",
                         (double)bb_width,
                         (unsigned)sm->tof_distance_mm);
            }
        } else {
            sm->lose_counter++;
            if (sm->lose_counter >= sm->max_lose_frames) {
                sm->lose_counter = 0;
                prv_enter_scan_retry(sm);
                SM_PRINT("[SM] CENTERED -> retry=%u  (lost target)\r\n",
                         (unsigned)sm->scan_retry);
            }
        }
        break;

    /* ================================================================
     *  APPROACH — move arm toward target
     *
     *  Move servo1 -5° and servo2 +5° per step (keep camera level).
     *  Each step → CALIBRATE to re-centre.
     *  Loop until CENTERED detects enough width → switches to CATCHING.
     * ================================================================ */
    case STATE_APPROACH:
        /* Move one approach step: s1 -4°, s2 +4° */
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

            delay_ms(150); // settle after movement

            sm->approach_step_count++;

            /* Reset sliding window — fresh 5 frames after each move */
            sm->calib_buf_idx = 0;
            sm->calib_sample_count = 0;
            sm->state = STATE_CALIBRATE;
            SM_PRINT("[SM] APPROACH -> CALIBRATE  (re-centre after step %u)\r\n",
                     (unsigned)sm->approach_step_count);
        }
        break;

    /* ================================================================
     *  CATCHING — pickup sequence
     *
     *  1. Close gripper gradually (缓慢闭合)
     *  2. servo2→130°, servo1→100° (抬臂 + 抬头)
     *  3. servo1→90° (return to vertical)
     *  4. servo0→-135° (rotate base to drop position)
     *  5. servo2→90° (tilt forward to drop)
     *  6. Open gripper (松开夹爪放下目标)
     * ================================================================ */
    case STATE_CATCHING: {
        SM_PRINT("[SM] CATCHING: starting pickup sequence\r\n");

        /* Tilt servo2 up a bit before pickup to clear the target */
        {
            uint8_t new_s2 = clamp_u8((int32_t)sm->cur_servo2 + 10, 0, 180);
            servo2_set_angle(new_s2);
            sm->cur_servo2 = new_s2;
            delay_ms(100);
        }

        /* Close gripper gradually (缓慢闭合) */
        servo_catch_close_slow(600);
        delay_ms(200); // wait for gripper to close

        /* Step 3: servo1 → 90° (return to vertical) */
        smooth_move_servo1(sm->cur_servo1, 90, NORMAL_STEP_MS);
        sm->cur_servo1 = 90;
        delay_ms(100);

        /* Step 4: servo0 → current - 45° (rotate base away) */
        {
            int32_t new_s0 = clamp_i32((int32_t)sm->cur_servo0 - 45, -135, 135);
            smooth_move_servo0(sm->cur_servo0, new_s0, FAST_STEP_MS);
            sm->cur_servo0 = new_s0;
        }
        delay_ms(100);

        /* Step 5: servo2 → current + 20° (tilt up to clear) */
        {
            uint8_t new_s2 = clamp_u8((int32_t)sm->cur_servo2 + 20, 0, 180);
            smooth_move_servo2(sm->cur_servo2, new_s2, FAST_STEP_MS);
            sm->cur_servo2 = new_s2;
        }
        delay_ms(100);

        /* Step 6: servo0 → 45° (return to centre) */
        smooth_move_servo0(sm->cur_servo0, 45, FAST_STEP_MS);
        sm->cur_servo0 = 45;
        delay_ms(100);

        /* Step 7: servo2 → 45° (tilt down to drop position) */
        smooth_move_servo2(sm->cur_servo2, 45, FAST_STEP_MS);
        sm->cur_servo2 = 45;
        delay_ms(100);

        /* Step 8: Open gripper (松开夹爪放下目标) */
        servo_catch_open();
        delay_ms(200); // wait for gripper to open

        SM_PRINT("[SM] CATCHING: pickup complete  -> HALTING\r\n");
        sm->state = STATE_HALTING;
        break;
    }

    /* ================================================================
     *  HALTING — restore arm to a scanning pose based on the pre-catch
     *  servo positions, then resume scanning.
     *
     *  servo0 → pre_catch_servo0 (restore recorded pan angle)
     *  servo1 → pre_catch_servo1 + (90 - pre_catch_servo1)/2
     *           (halfway back toward vertical)
     *  servo2 → pre_catch_servo2 - s1_delta
     *           (tilt down by same amount s1 moved up toward 90°)
     *  Then → SCAN_SERVO2 to continue searching.
     * ================================================================ */
    case STATE_HALTING: {
        /* Calculate target positions using pre-catch snapshot */
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

        /* Restore servo0 to pre-catch angle */
        smooth_move_servo0(sm->cur_servo0, sm->pre_catch_servo0, NORMAL_STEP_MS);
        sm->cur_servo0 = sm->pre_catch_servo0;

        /* Restore arm to scanning pose */
        smooth_move_servo1(sm->cur_servo1, (uint8_t)s1_target, NORMAL_STEP_MS);
        sm->cur_servo1 = (uint8_t)s1_target;

        smooth_move_servo2(sm->cur_servo2, s2_target, NORMAL_STEP_MS);
        sm->cur_servo2 = s2_target;

        delay_ms(500);

        SM_PRINT("[SM] HALTING: scanning pose reached -> SCAN_SERVO2\r\n");

        /* Resume scanning from current servo2 position */
        sm->scan_angle = (int32_t)sm->cur_servo2;
        sm->scan_step = 1;
        sm->scan_retry = 0;
        sm->state = STATE_SCAN_SERVO2;
        break;
    }
    } /* switch */
}
