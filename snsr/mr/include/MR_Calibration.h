#ifndef MR_CALIBRATION_H
#define MR_CALIBRATION_H

#include <stdint.h>
#include <stdbool.h>
#include "MR_AngleSensor.h"

typedef struct
{
    float sin_min;
    float sin_max;
    float cos_min;
    float cos_max;
    uint32_t samples;
} MR_CalibAccumulator_t;

void MR_CalibStart(MR_CalibAccumulator_t *a);
void MR_CalibUpdate(MR_CalibAccumulator_t *a, float sin_raw, float cos_raw);
bool MR_CalibFinish(const MR_CalibAccumulator_t *a, MR_Calib_t *calib);

#endif
