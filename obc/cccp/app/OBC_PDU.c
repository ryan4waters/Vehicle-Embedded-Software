#include "OBC_PDU.h"
#include "CCCP_Platform.h"

static OBC_PDU_State g_pdu = PDU_OFF;

void OBC_PDU_Init(void)
{
    g_pdu = PDU_WAIT_PLUG;
}

void OBC_PDU_10msTask(const CCCP_Status *s)
{
    switch (g_pdu)
    {
        case PDU_OFF:
            g_pdu = PDU_WAIT_PLUG;
            break;

        case PDU_WAIT_PLUG:
            if (s->plug_present)
                g_pdu = PDU_PLUGGED;
            break;

        case PDU_PLUGGED:
            if (!s->plug_present)
                g_pdu = PDU_WAIT_PLUG;
            else if (s->vehicle_ready)
                g_pdu = PDU_WAIT_BMS;
            break;

        case PDU_WAIT_BMS:
            if (!s->plug_present)
                g_pdu = PDU_WAIT_PLUG;
            else if (s->fault)
                g_pdu = PDU_FAULT;
            else if (s->bms_current_a > 0.0f)
                g_pdu = PDU_PRECHARGE;
            break;

        case PDU_PRECHARGE:
            if (!s->charge_ready)
                g_pdu = PDU_STOPPING;
            else
                g_pdu = PDU_CHARGING;
            break;

        case PDU_CHARGING:
            if (!s->charge_allowed)
                g_pdu = PDU_STOPPING;
            break;

        case PDU_STOPPING:
            CCCP_Platform_SetChargePowerEnable(false);
            if (!s->plug_present)
                g_pdu = PDU_WAIT_PLUG;
            else
                g_pdu = PDU_PLUGGED;
            break;

        case PDU_FAULT:
        default:
            CCCP_Platform_SetChargePowerEnable(false);
            if (!s->fault)
                g_pdu = PDU_PLUGGED;
            break;
    }
}

OBC_PDU_State OBC_PDU_GetState(void)
{
    return g_pdu;
}
