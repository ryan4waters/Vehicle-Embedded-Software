#include "CCCP_Platform_tc377.h"

/*
 * TC377 adapter.
 *
 * Typical implementation:
 *   1. GTM CMU clock.
 *   2. GTM TIM channel input selection + digital filter.
 *   3. Rising/falling edge capture.
 *   4. EVADC CP/CC channels.
 *   5. Convert ADC counts using board divider.
 *
 * Infineon iLLD can be used here. Keep IfxGtm_Tim_In / EVADC calls
 * out of the common layer.
 */

static volatile uint32_t g_cp_period_ticks;
static volatile uint32_t g_cp_high_ticks;
static volatile bool g_cp_capture_valid;

/* Example only: choose this after actual GTM clock configuration. */
static const uint32_t g_cp_capture_timer_hz = 1000000U;

void CCCP_Platform_Init(void)
{
    /*
     * TODO:
     *
     * IfxGtm_enable(&MODULE_GTM);
     * IfxGtm_Cmu_enableClocks(...);
     *
     * Configure selected GTM TIM channel and CP input pin.
     * Configure EVADC group/channel.
     * Configure interrupt/notification.
     *
     * Exact API calls depend on the installed iLLD version.
     */
}

float CCCP_Platform_ReadCpVoltage(void)
{
    /* TODO: read EVADC result and apply real PCB scaling. */
    return 12.0f;
}

float CCCP_Platform_ReadCcResistance(void)
{
    /* TODO: calculate resistance from CC ADC front-end. */
    return 220.0f;
}

CCCP_PlatformPwmCapture CCCP_Platform_GetCpPwmCapture(void)
{
    CCCP_PlatformPwmCapture r;

    r.period_ticks = g_cp_period_ticks;
    r.high_ticks = g_cp_high_ticks;
    r.timer_hz = g_cp_capture_timer_hz;
    r.valid = g_cp_capture_valid;

    return r;
}

void CCCP_Platform_RequestWakeup(bool enable)
{
    /* TODO: SCU/PMIC wake request or internal wake latch. */
    (void)enable;
}

void CCCP_Platform_SetChargePowerEnable(bool enable)
{
    /* TODO: connect to project PDU/PFC enable interface. */
    (void)enable;
}

float CCCP_Platform_GetBmsCurrentLimitA(void) { return 30.0f; }
float CCCP_Platform_GetBmsPowerLimitW(void) { return 6600.0f; }
float CCCP_Platform_GetGridVoltageRmsV(void) { return 220.0f; }
float CCCP_Platform_GetGridPowerFactor(void) { return 0.99f; }
float CCCP_Platform_GetThermalCurrentLimitA(void) { return 30.0f; }
float CCCP_Platform_GetOemCurrentLimitA(void) { return 30.0f; }
float CCCP_Platform_GetOemPowerLimitW(void) { return 6600.0f; }
