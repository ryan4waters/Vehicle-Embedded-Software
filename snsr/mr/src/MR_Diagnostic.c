#include "MR_Diagnostic.h"

void MR_DiagInit(MR_DiagState_t *s, uint16_t threshold)
{
    s->sin_range_debounce = 0u;
    s->cos_range_debounce = 0u;
    s->vector_debounce = 0u;
    s->plausibility_debounce = 0u;
    s->redundancy_debounce = 0u;
    s->threshold = threshold;
}

static uint16_t inc_sat(uint16_t x, uint16_t max)
{
    return (x < max) ? (uint16_t)(x + 1u) : max;
}

MR_FaultCode_t MR_DiagEvaluate(MR_DiagState_t *s, uint32_t d)
{
    if (d & (MR_DIAG_SIN_RANGE | MR_DIAG_COS_RANGE))
        return MR_FAULT_SIGNAL_OPEN;

    if (d & MR_DIAG_VECTOR_LOW)
        s->vector_debounce = inc_sat(s->vector_debounce, s->threshold);

    if (d & MR_DIAG_VECTOR_HIGH)
        s->vector_debounce = inc_sat(s->vector_debounce, s->threshold);

    if (d & MR_DIAG_IMPLAUSIBLE)
        s->plausibility_debounce = inc_sat(s->plausibility_debounce, s->threshold);

    if (s->vector_debounce >= s->threshold)
        return MR_FAULT_VECTOR_LOW;

    if (s->plausibility_debounce >= s->threshold)
        return MR_FAULT_OVERSPEED;

    return MR_FAULT_NONE;
}
