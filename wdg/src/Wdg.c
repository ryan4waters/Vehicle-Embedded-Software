#include "Wdg.h"
#include "Wdg_Internal.h"
#include "Wdg_External.h"

static WdgContext g_wdg;

void Wdg_Init(void)
{
    g_wdg.appAlive    = 0U;
    g_wdg.dcdcAlive   = 0U;
    g_wdg.canAlive    = 0U;
    g_wdg.safetyAlive = 0U;

    g_wdg.lastAppAlive    = 0U;
    g_wdg.lastDcdcAlive   = 0U;
    g_wdg.lastCanAlive    = 0U;
    g_wdg.lastSafetyAlive = 0U;

    g_wdg.fault = WDG_FAULT_NONE;
    g_wdg.state = WDG_STATE_INIT;

    Wdg_Internal_Init();
    Wdg_External_Init();

    g_wdg.state = WDG_STATE_RUNNING;
}

void Wdg_MainFunction(void)
{
    uint32_t healthy = 1U;

    if(g_wdg.appAlive == g_wdg.lastAppAlive)
    {
        g_wdg.fault |= WDG_FAULT_APP;
        healthy = 0U;
    }

    if(g_wdg.dcdcAlive == g_wdg.lastDcdcAlive)
    {
        g_wdg.fault |= WDG_FAULT_DCDC;
        healthy = 0U;
    }

    if(g_wdg.canAlive == g_wdg.lastCanAlive)
    {
        g_wdg.fault |= WDG_FAULT_CAN;
        healthy = 0U;
    }

    if(g_wdg.safetyAlive == g_wdg.lastSafetyAlive)
    {
        g_wdg.fault |= WDG_FAULT_SAFETY;
        healthy = 0U;
    }

    g_wdg.lastAppAlive =
        g_wdg.appAlive;

    g_wdg.lastDcdcAlive =
        g_wdg.dcdcAlive;

    g_wdg.lastCanAlive =
        g_wdg.canAlive;

    g_wdg.lastSafetyAlive =
        g_wdg.safetyAlive;

    if(healthy)
    {
        Wdg_Internal_Service();
        Wdg_External_Service();
    }
}

void Wdg_NotifyApp(void)
{
    g_wdg.appAlive++;
}

void Wdg_NotifyDcdc(void)
{
    g_wdg.dcdcAlive++;
}

void Wdg_NotifyCan(void)
{
    g_wdg.canAlive++;
}

void Wdg_NotifySafety(void)
{
    g_wdg.safetyAlive++;
}

uint32_t Wdg_IsHealthy(void)
{
    return (g_wdg.fault == WDG_FAULT_NONE);
}