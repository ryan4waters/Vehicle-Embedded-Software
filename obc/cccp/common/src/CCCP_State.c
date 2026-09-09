#include "CCCP.h"
#include "CCCP_Cfg.h"
#include "CCCP_Platform.h"

static CCCP_State g_state;
static uint32_t g_plugged_ms;
static uint32_t g_ready_ms;
static uint32_t g_invalid_ms;

void CCCP_StateInit(void)
{
    g_state = CCCP_STATE_WAIT_PLUG;
    g_plugged_ms = 0U;
    g_ready_ms = 0U;
    g_invalid_ms = 0U;
}

void CCCP_StateUpdate(CCCP_Status *s)
{
    bool connected =
        (s->cp.state == CCCP_CP_B_9V) ||
        (s->cp.state == CCCP_CP_C_6V);

    bool ready =
        (s->cp.state == CCCP_CP_C_6V) &&
        s->cp.pwm_valid &&
        !s->cp.digital_comm_required;

    if (connected)
        g_plugged_ms += CCCP_TASK_10MS;
    else
        g_plugged_ms = 0U;

    if (ready)
        g_ready_ms += CCCP_TASK_10MS;
    else
        g_ready_ms = 0U;

    if (!s->cp.valid)
        g_invalid_ms += CCCP_TASK_10MS;
    else
        g_invalid_ms = 0U;

    s->plug_present =
        (g_plugged_ms >= CCCP_CP_CONFIRM_MS);

    s->vehicle_ready =
        (g_ready_ms >= CCCP_READY_CONFIRM_MS);

    if (g_invalid_ms >= CCCP_CP_INVALID_TIMEOUT_MS)
        s->fault = true;

    switch (g_state)
    {
        case CCCP_STATE_OFF:
            g_state = CCCP_STATE_WAIT_PLUG;
            break;

        case CCCP_STATE_WAIT_PLUG:
            if (s->plug_present)
                g_state = CCCP_STATE_PLUGGED;
            break;

        case CCCP_STATE_PLUGGED:
            if (!s->plug_present)
                g_state = CCCP_STATE_WAIT_PLUG;
            else if (s->vehicle_ready)
                g_state = CCCP_STATE_WAIT_BMS;
            break;

        case CCCP_STATE_WAIT_BMS:
            if (!s->plug_present)
                g_state = CCCP_STATE_WAIT_PLUG;
            else if (s->fault)
                g_state = CCCP_STATE_FAULT;
            else if (s->bms_current_a > 0.0f)
                g_state = CCCP_STATE_PRECHARGE;
            break;

        case CCCP_STATE_PRECHARGE:
            if (!s->plug_present || !s->vehicle_ready)
                g_state = CCCP_STATE_STOPPING;
            else if (!s->fault)
                g_state = CCCP_STATE_CHARGING;
            break;

        case CCCP_STATE_CHARGING:
            if (!s->plug_present || !s->vehicle_ready || s->fault)
                g_state = CCCP_STATE_STOPPING;
            break;

        case CCCP_STATE_STOPPING:
            if (!s->plug_present)
                g_state = CCCP_STATE_WAIT_PLUG;
            else if (!s->vehicle_ready)
                g_state = CCCP_STATE_PLUGGED;
            break;

        case CCCP_STATE_FAULT:
        default:
            if (!s->fault && s->plug_present)
                g_state = CCCP_STATE_PLUGGED;
            break;
    }

    s->charge_ready =
        (g_state == CCCP_STATE_WAIT_BMS) ||
        (g_state == CCCP_STATE_PRECHARGE) ||
        (g_state == CCCP_STATE_CHARGING);

    s->charge_allowed =
        (g_state == CCCP_STATE_PRECHARGE) ||
        (g_state == CCCP_STATE_CHARGING);

    CCCP_Platform_RequestWakeup(
        g_state != CCCP_STATE_WAIT_PLUG);

    CCCP_Platform_SetChargePowerEnable(
        s->charge_allowed && !s->fault);
}

CCCP_State CCCP_StateGet(void)
{
    return g_state;
}
