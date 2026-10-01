#include "MR_Calibration.h"

void MR_CalibStart(MR_CalibAccumulator_t *a)
{
    a->sin_min =  1.0e30f;
    a->sin_max = -1.0e30f;
    a->cos_min =  1.0e30f;
    a->cos_max = -1.0e30f;
    a->samples = 0u;
}

void MR_CalibUpdate(MR_CalibAccumulator_t *a, float s, float c)
{
    if (s < a->sin_min) a->sin_min = s;
    if (s > a->sin_max) a->sin_max = s;
    if (c < a->cos_min) a->cos_min = c;
    if (c > a->cos_max) a->cos_max = c;
    a->samples++;
}

bool MR_CalibFinish(const MR_CalibAccumulator_t *a, MR_Calib_t *calib)
{
    float s_amp, c_amp;

    if ((a->samples < 100u) ||
        (a->sin_max <= a->sin_min) ||
        (a->cos_max <= a->cos_min))
        return false;

    calib->sin_offset = 0.5f * (a->sin_max + a->sin_min);
    calib->cos_offset = 0.5f * (a->cos_max + a->cos_min);

    s_amp = 0.5f * (a->sin_max - a->sin_min);
    c_amp = 0.5f * (a->cos_max - a->cos_min);

    if ((s_amp < 1.0e-6f) || (c_amp < 1.0e-6f))
        return false;

    calib->sin_gain = 1.0f / s_amp;
    calib->cos_gain = 1.0f / c_amp;

    /* Can be replaced by ellipse/matrix fitting for high accuracy. */
    calib->orthogonality = 0.0f;
    calib->zero_offset_rad = 0.0f;
    calib->direction = 1.0f;

    return true;
}
