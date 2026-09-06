#include "WheelSpeed.h"
#include "WheelSpeed_Cfg.h"
#include "WheelCapture.h"
#include "WheelSpeed_AK.h"
#include "WheelSpeed_PWM.h"
#include <math.h>

static WheelSpeedData_t g_wheel[WHEEL_COUNT] =
{
    { WHEEL_SENSOR_AK,  WHEEL_PULSE_PER_REV_DEFAULT, WHEEL_CIRCUMFERENCE_M_DEFAULT },
    { WHEEL_SENSOR_AK,  WHEEL_PULSE_PER_REV_DEFAULT, WHEEL_CIRCUMFERENCE_M_DEFAULT },
    { WHEEL_SENSOR_PWM, WHEEL_PULSE_PER_REV_DEFAULT, WHEEL_CIRCUMFERENCE_M_DEFAULT },
    { WHEEL_SENSOR_PWM, WHEEL_PULSE_PER_REV_DEFAULT, WHEEL_CIRCUMFERENCE_M_DEFAULT }
};

static uint32_t g_nowTicks;

static float speed_from_frequency(float frequencyHz, float ppr, float circumferenceM)
{
    if ((ppr <= 0.0f) || (circumferenceM <= 0.0f))
    {
        return 0.0f;
    }

    return frequencyHz * 60.0f / ppr * circumferenceM * 0.06f;
}

static float speed_from_period(uint32_t periodTicks,
                               float ppr,
                               float circumferenceM)
{
    float f;

    if (periodTicks == 0U)
    {
        return 0.0f;
    }

    f = (float)WHEEL_TIMER_FREQ_HZ / (float)periodTicks;
    return speed_from_frequency(f, ppr, circumferenceM);
}

static void process_capture_events(uint8_t wheel)
{
    WheelCaptureEvent_t e;
    WheelSpeedData_t *w = &g_wheel[wheel];

    while (WheelCapture_GetEvent(wheel, &e))
    {
        w->lastEdgeTimestamp = e.timestamp;

        if (e.edge == WHEEL_EDGE_RISING)
        {
            w->risingEdgeCount++;
            w->pulseCount++;
        }
        else
        {
            w->fallingEdgeCount++;
        }
    }
}

/*
 * Generic M/T calculation from a local snapshot.
 *
 * T method:
 *   speed is obtained from time between adjacent pulses.
 *   Excellent resolution at low speed, poor update/quantization behavior
 *   when the period becomes very long.
 *
 * M method:
 *   count N pulses during fixed gate Tg.
 *   f = N/Tg.
 *   Better statistical behavior and update rate at high speed.
 */
static float calc_T(uint32_t periodTicks, WheelSpeedData_t *w)
{
    if ((periodTicks < WHEEL_MIN_PERIOD_US) ||
        (periodTicks > WHEEL_MAX_PERIOD_US))
    {
        return -1.0f;
    }

    return speed_from_period(periodTicks,
                             w->pulsePerRev,
                             w->circumferenceM);
}

static float calc_M(uint32_t pulseCount, float gateTimeS,
                    WheelSpeedData_t *w)
{
    float f;

    if (gateTimeS <= 0.0f)
    {
        return -1.0f;
    }

    if (pulseCount == 0U)
    {
        return 0.0f;
    }

    f = (float)pulseCount / gateTimeS;

    return speed_from_frequency(f,
                                w->pulsePerRev,
                                w->circumferenceM);
}

static float select_MT(float tSpeed, float mSpeed, uint32_t pulseCount,
                       WheelSpeedData_t *w)
{
    /*
     * Adaptive rule:
     * - At very low speed, use T: M has too few pulses per gate.
     * - At high speed, use M: averaging many pulses reduces quantization.
     * - In the transition region, blend the two measurements.
     *
     * This is a practical implementation; tune thresholds with vehicle data.
     */
    float reference = (tSpeed >= 0.0f) ? tSpeed : mSpeed;

    if (reference < WHEEL_MT_LOW_SPEED_KPH)
    {
        w->source = WHEEL_MEAS_T;
        return tSpeed;
    }

    if ((reference > WHEEL_MT_HIGH_SPEED_KPH) && (pulseCount >= 2U))
    {
        w->source = WHEEL_MEAS_M;
        return mSpeed;
    }

    if ((tSpeed >= 0.0f) && (mSpeed >= 0.0f))
    {
        float alpha = 0.5f;

        w->source = WHEEL_MEAS_T;
        (void)pulseCount;

        /* In transition, average M/T to avoid a hard switch. */
        return alpha * tSpeed + (1.0f - alpha) * mSpeed;
    }

    w->source = (tSpeed >= 0.0f) ?
                WHEEL_MEAS_T : WHEEL_MEAS_M;

    return reference;
}

static void kalman_update(uint8_t wheel)
{
    WheelSpeedData_t *w = &g_wheel[wheel];
    float measurement;
    float r;
    float dt;

    if (!w->valid)
    {
        return;
    }

    measurement = w->rawSpeedKph;

    /*
     * Speed-dependent R:
     * lower speeds are intrinsically noisier with pulse timing, so increase R.
     */
    if (measurement < WHEEL_MT_LOW_SPEED_KPH)
    {
        r = WHEEL_KF_R_LOW_SPEED;
    }
    else if (measurement > WHEEL_MT_HIGH_SPEED_KPH)
    {
        r = WHEEL_KF_R_HIGH_SPEED;
    }
    else
    {
        float ratio =
            (measurement - WHEEL_MT_LOW_SPEED_KPH) /
            (WHEEL_MT_HIGH_SPEED_KPH - WHEEL_MT_LOW_SPEED_KPH);

        r = WHEEL_KF_R_LOW_SPEED +
            ratio * (WHEEL_KF_R_HIGH_SPEED - WHEEL_KF_R_LOW_SPEED);
    }

    dt = WHEEL_SPEED_PERIOD_S;

    w->filteredSpeedKph =
        WheelKalman_Update(&w->kalman,
                           measurement,
                           dt,
                           r);
}

void WheelSpeed_Init(void)
{
    WheelCapture_Init();
    WheelSpeed_AK_Init();
    WheelSpeed_PWM_Init();

    for (uint8_t i = 0U; i < WHEEL_COUNT; ++i)
    {
        WheelKalman_Init(&g_wheel[i].kalman, 0.0f);
        g_wheel[i].status = WHEEL_STATUS_INIT;
        g_wheel[i].valid = false;
        g_wheel[i].timeout = false;
    }
}

void WheelSpeed_MainFunction(void)
{
    /*
     * In a real project g_nowTicks must be read from the same free-running
     * time base used by capture. This template increments it conceptually;
     * replace with the actual eMIOS/GTM counter read.
     */
    g_nowTicks += (uint32_t)(WHEEL_TIMER_FREQ_HZ *
                             WHEEL_SPEED_PERIOD_S);

    for (uint8_t i = 0U; i < WHEEL_COUNT; ++i)
    {
        WheelSpeedData_t *w = &g_wheel[i];

        if (w->sensorType == WHEEL_SENSOR_AK)
        {
            WheelSpeed_AK_Process(i);
        }
        else
        {
            WheelSpeed_PWM_Process(i);
        }

        process_capture_events(i);

        /*
         * The project-specific AK/PWM parser should expose the latest
         * period/pulse-count snapshot here. For the generic template,
         * periodTicks is expected to be filled by the parser/MCAL adapter.
         */
        if (w->periodTicks != 0U)
        {
            float tSpeed = calc_T(w->periodTicks, w);
            float mSpeed = calc_M(w->pulseCount,
                                  WHEEL_M_GATE_TIME_S,
                                  w);

            float selected;

#if (WHEEL_MEASUREMENT_METHOD == WHEEL_METHOD_T)
            selected = tSpeed;
            w->source = WHEEL_MEAS_T;
#elif (WHEEL_MEASUREMENT_METHOD == WHEEL_METHOD_M)
            selected = mSpeed;
            w->source = WHEEL_MEAS_M;
#else
            selected = select_MT(tSpeed, mSpeed,
                                 w->pulseCount, w);
#endif

            if (selected >= 0.0f)
            {
                w->rawSpeedKph = selected;
                w->rpm = (w->pulsePerRev > 0.0f) ?
                         (selected / (w->circumferenceM * 0.06f)) :
                         0.0f;
                w->frequencyHz =
                    (w->rpm * w->pulsePerRev) / 60.0f;

                w->valid = true;
                w->status = WHEEL_STATUS_VALID;
                w->timeout = false;
                w->implausible = false;

                kalman_update(i);
            }
        }

        if ((g_nowTicks - w->lastEdgeTimestamp) >
            (uint32_t)((uint64_t)WHEEL_TIMEOUT_US *
                       WHEEL_TIMER_FREQ_HZ / 1000000ULL))
        {
            w->timeout = true;
            w->valid = false;
            w->status = WHEEL_STATUS_TIMEOUT;
        }
    }
}

const WheelSpeedData_t *WheelSpeed_Get(uint8_t wheel)
{
    if (wheel >= WHEEL_COUNT)
    {
        return NULL;
    }

    return &g_wheel[wheel];
}
