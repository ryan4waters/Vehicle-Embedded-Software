#ifndef MR_DIAGNOSTIC_H
#define MR_DIAGNOSTIC_H

#include <stdint.h>
#include <stdbool.h>
#include "MR_AngleSensor.h"

typedef enum
{
    MR_FAULT_NONE = 0,
    MR_FAULT_SIGNAL_OPEN,
    MR_FAULT_SIGNAL_SHORT_GND,
    MR_FAULT_SIGNAL_SHORT_VBAT,
    MR_FAULT_VECTOR_LOW,
    MR_FAULT_VECTOR_HIGH,
    MR_FAULT_OVERSPEED,
    MR_FAULT_REDUNDANCY,
    MR_FAULT_CALIBRATION
} MR_FaultCode_t;

typedef struct
{
    uint16_t sin_range_debounce;
    uint16_t cos_range_debounce;
    uint16_t vector_debounce;
    uint16_t plausibility_debounce;
    uint16_t redundancy_debounce;
    uint16_t threshold;
} MR_DiagState_t;

void MR_DiagInit(MR_DiagState_t *s, uint16_t threshold);
MR_FaultCode_t MR_DiagEvaluate(MR_DiagState_t *s, uint32_t diag_bits);

#endif
