#include "CCCP_Cfg.h"
#include "CCCP_Types.h"
#include <math.h>

static CCCP_CpState decode_cp_voltage(float v)
{
    if ((v >= CCCP_CP_A_MIN_V) && (v <= CCCP_CP_A_MAX_V))
        return CCCP_CP_A_12V;

    if ((v >= CCCP_CP_B_MIN_V) && (v < CCCP_CP_B_MAX_V))
        return CCCP_CP_B_9V;

    if ((v >= CCCP_CP_C_MIN_V) && (v < CCCP_CP_C_MAX_V))
        return CCCP_CP_C_6V;

    if ((v >= CCCP_CP_0V_MIN_V) && (v <= CCCP_CP_0V_MAX_V))
        return CCCP_CP_FAULT_0V;

    if (v < CCCP_CP_NEG12_MAX_V)
        return CCCP_CP_NEG_12V;

    return CCCP_CP_UNKNOWN;
}

static float decode_cp_current(float duty, bool *digital_comm)
{
    float duty_pct = duty * 100.0f;
    *digital_comm = false;

    if ((duty >= CCCP_CP_PWM_5PCT_MIN) &&
        (duty <= CCCP_CP_PWM_5PCT_MAX))
    {
        *digital_comm = true;
        return 0.0f;
    }

    if ((duty_pct >= 10.0f) && (duty_pct <= 85.0f))
        return duty_pct * 0.6f;

    if ((duty_pct > 85.0f) && (duty_pct <= 90.0f))
    {
        float current = (duty_pct - 64.0f) * 2.5f;
        return (current > 63.0f) ? 63.0f : current;
    }

    return 0.0f;
}

static float decode_cc_current(float r)
{
    float err;

    if (r <= 0.0f)
        return 0.0f;

    err = CCCP_CC_TOL_PCT / 100.0f;

    if (fabsf(r - CCCP_CC_R_680_OHM) <=
        CCCP_CC_R_680_OHM * err)
        return 20.0f;

    if (fabsf(r - CCCP_CC_R_220_OHM) <=
        CCCP_CC_R_220_OHM * err)
        return 32.0f;

    if (fabsf(r - CCCP_CC_R_100_OHM) <=
        CCCP_CC_R_100_OHM * err)
        return 63.0f;

    return 0.0f;
}

void CCCP_DecodeCp(CCCP_CpInfo *cp)
{
    cp->state = decode_cp_voltage(cp->voltage_v);

    if (cp->pwm_valid)
        cp->evse_current_a =
            decode_cp_current(cp->pwm.duty,
                              &cp->digital_comm_required);
    else
    {
        cp->evse_current_a = 0.0f;
        cp->digital_comm_required = false;
    }
}

void CCCP_DecodeCc(CCCP_CcInfo *cc)
{
    cc->cable_current_a =
        decode_cc_current(cc->resistance_ohm);
}
