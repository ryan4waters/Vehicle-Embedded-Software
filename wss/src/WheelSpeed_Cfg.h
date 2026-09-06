#ifndef WHEEL_SPEED_CFG_H
#define WHEEL_SPEED_CFG_H

#include <stdint.h>
#include <stdbool.h>

/* Project configuration: replace with real vehicle/sensor values. */
#define WHEEL_COUNT                    (4U)
#define WHEEL_TIMER_FREQ_HZ            (1000000UL)   /* 1 MHz => 1 tick/us */
#define WHEEL_TIMER_MAX_COUNT          (0xFFFFFFFFUL)

#define WHEEL_CONTROL_PERIOD_S         (0.001f)      /* 1 ms */
#define WHEEL_SPEED_PERIOD_S            (0.001f)

/* Select measurement method:
 *  M  : pulse count in a fixed gate time
 *  T  : time between adjacent edges/pulses
 *  MT : adaptive M/T fusion
 */
#define WHEEL_METHOD_M                 (0U)
#define WHEEL_METHOD_T                 (1U)
#define WHEEL_METHOD_MT                (2U)

#define WHEEL_MEASUREMENT_METHOD       WHEEL_METHOD_MT

/* Pulse/geometry calibration. */
#define WHEEL_PULSE_PER_REV_DEFAULT    (48.0f)
#define WHEEL_CIRCUMFERENCE_M_DEFAULT  (2.0f)

/* Adaptive M/T thresholds. Below this speed, T is normally preferred;
 * above this speed, M becomes useful because enough pulses exist in the gate.
 */
#define WHEEL_MT_LOW_SPEED_KPH         (5.0f)
#define WHEEL_MT_HIGH_SPEED_KPH        (20.0f)

/* Gate for M method. */
#define WHEEL_M_GATE_TIME_S            (0.010f)      /* 10 ms */

/* Signal validity limits. */
#define WHEEL_MIN_PERIOD_US            (20U)
#define WHEEL_MAX_PERIOD_US            (2000000U)
#define WHEEL_TIMEOUT_US               (2500000U)

/* Measurement noise / plausibility. */
#define WHEEL_MAX_ACCEL_KPH_PER_S      (80.0f)
#define WHEEL_MAX_JUMP_KPH             (20.0f)

/* Kalman filter tuning.
 * State x = [speed, acceleration]^T.
 * Q controls trust in dynamic model; R controls trust in measurement.
 */
#define WHEEL_KF_Q_SPEED               (0.20f)
#define WHEEL_KF_Q_ACCEL               (20.0f)
#define WHEEL_KF_R_SPEED               (4.0f)

/* Optional speed-dependent measurement noise. */
#define WHEEL_KF_R_LOW_SPEED           (9.0f)
#define WHEEL_KF_R_HIGH_SPEED          (1.5f)

#endif
