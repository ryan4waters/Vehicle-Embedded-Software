#include "CCCP_Platform.h"
#include "CCCP_Cfg.h"
#include "CCCP_Types.h"

static float minf(float a, float b)
{
    return (a < b) ? a : b;
}

void CCCP_UpdateLimits(CCCP_Status *s)
{
    float limit = s->cp.evse_current_a;

    /*
     * The CC decoder returns 0 when the example table has no match.
     * In a real project, "invalid CC" should normally be a state/fault,
     * not silently interpreted as zero. This package keeps the behavior
     * explicit: invalid/unknown cable capacity blocks charging.
     */
    if (s->cc.cable_current_a <= 0.0f)
    {
        s->current_limit_a = 0.0f;
        s->power_limit_w = 0.0f;
        return;
    }

    limit = minf(limit, s->cc.cable_current_a);
    limit = minf(limit, s->bms_current_a);
    limit = minf(limit, s->thermal_current_a);
    limit = minf(limit, s->oem_current_a);
    limit = minf(limit, s->obc_current_a);

    if (limit < 0.0f)
        limit = 0.0f;

    s->current_limit_a = limit;

    float vac = CCCP_Platform_GetGridVoltageRmsV();
    float pf = CCCP_Platform_GetGridPowerFactor();

    float p_from_i = limit * vac * pf;
    float p = minf(p_from_i, s->bms_power_w);
    p = minf(p, s->oem_power_w);
    p = minf(p, s->obc_power_w);

    s->power_limit_w = (p > 0.0f) ? p : 0.0f;
}
