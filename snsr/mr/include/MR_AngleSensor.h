#ifndef MR_ANGLE_SENSOR_H
#define MR_ANGLE_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    float sin_offset;
    float cos_offset;
    float sin_gain;
    float cos_gain;
    float orthogonality;      /* rad; 0 = ideal */
    float zero_offset_rad;    /* mechanical/electrical zero */
    float direction;          /* +1.0 or -1.0 */
} MR_Calib_t;

typedef struct
{
    float sin_raw;
    float cos_raw;
    float sin_cal;
    float cos_cal;
    float angle_rad;
    float angle_deg;
    float angle_unwrapped_deg;
    float speed_rpm;
    uint32_t diag;
} MR_Output_t;

typedef struct
{
    float prev_angle_deg;
    float sample_time_s;
    uint8_t initialized;
} MR_State_t;

typedef struct
{
    MR_Calib_t calib;
    MR_State_t state;
} MR_AngleSensor_t;

typedef enum
{
    MR_DIAG_OK             = 0u,
    MR_DIAG_SIN_RANGE      = 1u << 0,
    MR_DIAG_COS_RANGE      = 1u << 1,
    MR_DIAG_VECTOR_LOW     = 1u << 2,
    MR_DIAG_VECTOR_HIGH    = 1u << 3,
    MR_DIAG_IMPLAUSIBLE    = 1u << 4,
    MR_DIAG_REDUNDANCY     = 1u << 5,
    MR_DIAG_SENSOR_POWER   = 1u << 6,
    MR_DIAG_CALIB_INVALID  = 1u << 7
} MR_Diag_t;

typedef struct
{
    float sin_min;
    float sin_max;
    float cos_min;
    float cos_max;
    float min_vector;
    float max_vector;
    float max_speed_rpm;
    float redundancy_tol_deg;
} MR_Limits_t;

void MR_Init(MR_AngleSensor_t *ctx, const MR_Calib_t *calib, float sample_time_s);
void MR_Process(MR_AngleSensor_t *ctx, float sin_raw, float cos_raw,
                MR_Output_t *out, const MR_Limits_t *limits);
float MR_Atan2Deg(float y, float x);
float MR_Wrap360(float deg);
float MR_Wrap180(float deg);
bool MR_LearnZero(MR_AngleSensor_t *ctx, float reference_angle_deg,
                  float measured_angle_deg, float max_error_deg);

#endif
