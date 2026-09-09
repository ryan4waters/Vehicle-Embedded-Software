#include "CCCP.h"
#include "CCCP_Cfg.h"
#include "CCCP_Platform.h"

extern void CCCP_MeasureCp(CCCP_CpInfo *cp);
extern void CCCP_MeasureCc(CCCP_CcInfo *cc);
extern void CCCP_DecodeCp(CCCP_CpInfo *cp);
extern void CCCP_DecodeCc(CCCP_CcInfo *cc);
extern void CCCP_UpdateLimits(CCCP_Status *s);
extern void CCCP_StateInit(void);
extern void CCCP_StateUpdate(CCCP_Status *s);
extern CCCP_State CCCP_StateGet(void);

static CCCP_Status g_status;

void CCCP_Init(void)
{
    CCCP_Platform_Init();

    g_status = (CCCP_Status){0};

    g_status.obc_current_a = CCCP_OBC_RATED_CURRENT_A;
    g_status.obc_power_w = CCCP_OBC_RATED_POWER_W;
    g_status.oem_current_a = CCCP_OBC_RATED_CURRENT_A;
    g_status.oem_power_w = CCCP_OBC_RATED_POWER_W;
    g_status.thermal_current_a = CCCP_OBC_RATED_CURRENT_A;

    CCCP_StateInit();
}

void CCCP_1msTask(void)
{
    CCCP_MeasureCp(&g_status.cp);
    CCCP_MeasureCc(&g_status.cc);
}

void CCCP_10msTask(void)
{
    g_status.bms_current_a =
        CCCP_Platform_GetBmsCurrentLimitA();

    g_status.bms_power_w =
        CCCP_Platform_GetBmsPowerLimitW();

    g_status.thermal_current_a =
        CCCP_Platform_GetThermalCurrentLimitA();

    g_status.oem_current_a =
        CCCP_Platform_GetOemCurrentLimitA();

    g_status.oem_power_w =
        CCCP_Platform_GetOemPowerLimitW();

    CCCP_DecodeCp(&g_status.cp);
    CCCP_DecodeCc(&g_status.cc);

    CCCP_UpdateLimits(&g_status);
    CCCP_StateUpdate(&g_status);
}

const CCCP_Status *CCCP_GetStatus(void)
{
    return &g_status;
}

CCCP_State CCCP_GetState(void)
{
    return CCCP_StateGet();
}

bool CCCP_IsPlugged(void)
{
    return g_status.plug_present;
}

bool CCCP_IsVehicleReady(void)
{
    return g_status.vehicle_ready;
}

bool CCCP_IsChargeAllowed(void)
{
    return g_status.charge_allowed;
}
