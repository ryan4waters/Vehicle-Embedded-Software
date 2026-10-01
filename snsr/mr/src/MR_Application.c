/*
 * Reference application flow
 *
 * ADC ISR / DMA completion:
 *   1. obtain synchronous SIN/COS ADC values
 *   2. convert ADC code -> volts or normalized sensor unit
 *   3. MR_Process()
 *
 * 1ms/2ms task:
 *   4. evaluate diagnostic result
 *   5. publish angle/speed/fault
 *
 * Important:
 *   ADC sampling of SIN/COS should be synchronized as tightly as possible.
 */

#include "MR_AngleSensor.h"
#include "MR_Calibration.h"
#include "MR_Diagnostic.h"

static MR_AngleSensor_t g_mr;
static MR_DiagState_t g_diag;

void MR_App_Init(void)
{
    MR_Calib_t c = {
        .sin_offset = 0.0f,
        .cos_offset = 0.0f,
        .sin_gain = 1.0f,
        .cos_gain = 1.0f,
        .orthogonality = 0.0f,
        .zero_offset_rad = 0.0f,
        .direction = 1.0f
    };

    MR_Limits_t limits = {
        .sin_min = -1.5f,
        .sin_max =  1.5f,
        .cos_min = -1.5f,
        .cos_max =  1.5f,
        .min_vector = 0.30f,
        .max_vector = 1.70f,
        .max_speed_rpm = 20000.0f,
        .redundancy_tol_deg = 5.0f
    };

    (void)limits;
    MR_Init(&g_mr, &c, 0.001f);
    MR_DiagInit(&g_diag, 5u);
}

/* Call after ADC/DMA has a coherent SIN/COS pair. */
void MR_App_1ms(float sin_value, float cos_value)
{
    MR_Output_t out;
    MR_Limits_t limits = {
        -1.5f, 1.5f, -1.5f, 1.5f,
        0.30f, 1.70f, 20000.0f, 5.0f
    };

    MR_Process(&g_mr, sin_value, cos_value, &out, &limits);

    /* Production software:
       - store out.angle_deg / speed_rpm
       - run diagnostic debounce
       - update safety state
       - optionally compare redundant channel
       - do not allow a single bad sample to trip a safety fault
    */
    (void)out;
}
