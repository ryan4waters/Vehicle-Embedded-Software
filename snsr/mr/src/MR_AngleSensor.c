#include "MR_AngleSensor.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

static float clampf(float x, float lo, float hi)
{
    return (x < lo) ? lo : ((x > hi) ? hi : x);
}

float MR_Wrap360(float deg)
{
    while (deg >= 360.0f) deg -= 360.0f;
    while (deg < 0.0f) deg += 360.0f;
    return deg;
}

float MR_Wrap180(float deg)
{
    while (deg > 180.0f) deg -= 360.0f;
    while (deg <= -180.0f) deg += 360.0f;
    return deg;
}

float MR_Atan2Deg(float y, float x)
{
    return atan2f(y, x) * 180.0f / M_PI;
}

void MR_Init(MR_AngleSensor_t *ctx, const MR_Calib_t *calib, float sample_time_s)
{
    ctx->calib = *calib;
    ctx->state.prev_angle_deg = 0.0f;
    ctx->state.sample_time_s = sample_time_s;
    ctx->state.initialized = 0u;
}

void MR_Process(MR_AngleSensor_t *ctx, float sin_raw, float cos_raw,
                MR_Output_t *out, const MR_Limits_t *limits)
{
    float s, c, angle, delta;
    float mag2;

    out->diag = MR_DIAG_OK;
    out->sin_raw = sin_raw;
    out->cos_raw = cos_raw;

    if ((sin_raw < limits->sin_min) || (sin_raw > limits->sin_max))
        out->diag |= MR_DIAG_SIN_RANGE;

    if ((cos_raw < limits->cos_min) || (cos_raw > limits->cos_max))
        out->diag |= MR_DIAG_COS_RANGE;

    s = (sin_raw - ctx->calib.sin_offset) * ctx->calib.sin_gain;
    c = (cos_raw - ctx->calib.cos_offset) * ctx->calib.cos_gain;

    /* First-order orthogonality compensation.
       For production projects this can be replaced by matrix calibration. */
    s = s - ctx->calib.orthogonality * c;
    out->sin_cal = s;
    out->cos_cal = c;

    mag2 = s*s + c*c;

    if (mag2 < limits->min_vector * limits->min_vector)
        out->diag |= MR_DIAG_VECTOR_LOW;
    if (mag2 > limits->max_vector * limits->max_vector)
        out->diag |= MR_DIAG_VECTOR_HIGH;

    angle = MR_Atan2Deg(s, c);
    angle = MR_Wrap360(ctx->calib.direction * angle);
    angle = MR_Wrap360(angle - ctx->calib.zero_offset_rad * 180.0f / M_PI);

    out->angle_deg = angle;
    out->angle_rad = angle * M_PI / 180.0f;

    if (!ctx->state.initialized)
    {
        ctx->state.prev_angle_deg = angle;
        ctx->state.initialized = 1u;
        out->angle_unwrapped_deg = angle;
        out->speed_rpm = 0.0f;
        return;
    }

    delta = MR_Wrap180(angle - ctx->state.prev_angle_deg);
    out->angle_unwrapped_deg = ctx->state.prev_angle_deg + delta;
    out->speed_rpm = (delta / 360.0f) / ctx->state.sample_time_s * 60.0f;

    if (fabsf(out->speed_rpm) > limits->max_speed_rpm)
        out->diag |= MR_DIAG_IMPLAUSIBLE;

    ctx->state.prev_angle_deg = angle;
}

bool MR_LearnZero(MR_AngleSensor_t *ctx, float reference_angle_deg,
                  float measured_angle_deg, float max_error_deg)
{
    float err = MR_Wrap180(measured_angle_deg - reference_angle_deg);

    if (fabsf(err) > max_error_deg)
        return false;

    ctx->calib.zero_offset_rad += err * M_PI / 180.0f;
    while (ctx->calib.zero_offset_rad >= 2.0f*M_PI)
        ctx->calib.zero_offset_rad -= 2.0f*M_PI;
    while (ctx->calib.zero_offset_rad < 0.0f)
        ctx->calib.zero_offset_rad += 2.0f*M_PI;

    return true;
}
