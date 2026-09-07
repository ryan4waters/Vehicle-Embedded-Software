#include "WheelSpeed_Filter.h"
#include "WheelSpeed_Cfg.h"

static float clampf_local(float x, float lo, float hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

void WheelKalman_Init(WheelKalman_t *kf, float initialSpeedKph)
{
    if (kf == NULL)
    {
        return;
    }

    kf->speedKph = initialSpeedKph;
    kf->accelerationKphPerS = 0.0f;

    kf->p00 = 25.0f;
    kf->p01 = 0.0f;
    kf->p10 = 0.0f;
    kf->p11 = 100.0f;

    kf->initialized = true;
}

float WheelKalman_Update(WheelKalman_t *kf,
                         float z,
                         float dt,
                         float measurementNoise)
{
    float x0, x1;
    float p00, p01, p10, p11;
    float q00, q01, q11;
    float y, s, k0, k1;
    float dt2, dt3, dt4;

    if ((kf == NULL) || !kf->initialized)
    {
        return z;
    }

    dt = clampf_local(dt, 0.0001f, 0.100f);
    measurementNoise = clampf_local(measurementNoise, 0.01f, 1000.0f);

    /* Prediction:
     * x(k+1) = A*x(k)
     * [v] = [1 dt][v]
     * [a]   [0  1][a]
     */
    x0 = kf->speedKph + dt * kf->accelerationKphPerS;
    x1 = kf->accelerationKphPerS;

    p00 = kf->p00 + dt * (kf->p10 + kf->p01) + dt * dt * kf->p11;
    p01 = kf->p01 + dt * kf->p11;
    p10 = kf->p10 + dt * kf->p11;
    p11 = kf->p11;

    /* Simple tunable process-noise model. */
    dt2 = dt * dt;
    dt3 = dt2 * dt;
    dt4 = dt3 * dt;

    q00 = WHEEL_KF_Q_SPEED * dt4 * 0.25f;
    q01 = WHEEL_KF_Q_SPEED * dt3 * 0.5f;
    q11 = WHEEL_KF_Q_ACCEL * dt2;

    p00 += q00;
    p01 += q01;
    p10 += q01;
    p11 += q11;

    /* Measurement: z = speed + noise. */
    y = z - x0;
    s = p00 + measurementNoise;

    if (s < 1.0e-9f)
    {
        return x0;
    }

    k0 = p00 / s;
    k1 = p10 / s;

    x0 += k0 * y;
    x1 += k1 * y;

    p00 = (1.0f - k0) * p00;
    p01 = (1.0f - k0) * p01;
    p10 = p10 - k1 * p00;
    p11 = p11 - k1 * p01;

    kf->speedKph = clampf_local(x0, 0.0f, 400.0f);
    kf->accelerationKphPerS =
        clampf_local(x1, -WHEEL_MAX_ACCEL_KPH_PER_S,
                          WHEEL_MAX_ACCEL_KPH_PER_S);

    kf->p00 = p00;
    kf->p01 = p01;
    kf->p10 = p10;
    kf->p11 = p11;

    return kf->speedKph;
}
