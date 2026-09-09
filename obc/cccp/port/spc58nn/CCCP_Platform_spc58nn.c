#include "CCCP_Platform_spc58nn.h"

/*
 * SPC58NN adapter.
 *
 * Exact timer instance/channel and MCAL API depend on the selected
 * SPC58NN derivative, MCAL release and project configuration.
 */

static volatile uint32_t g_cp_period_ticks;
static volatile uint32_t g_cp_high_ticks;
static volatile bool g_cp_capture_valid;

static const uint32_t g_cp_capture_timer_hz = 1000000U;

void CCCP_Platform_Init(void)
{
    /*
     * TODO:
     *   - ADC/MCAL configuration
     *   - ICU/input capture configuration
     *   - PORT pinmux
     *   - IRQ callback
     *   - wakeup input
     */
}

float CCCP_Platform_ReadCpVoltage(void)
{
    return 12.0f; /* TODO */
}

float CCCP_Platform_ReadCcResistance(void)
{
    return 220.0f; /* TODO */
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
    /* TODO */
    (void)enable;
}

void CCCP_Platform_SetChargePowerEnable(bool enable)
{
    /* TODO */
    (void)enable;
}

float CCCP_Platform_GetBmsCurrentLimitA(void) { return 30.0f; }
float CCCP_Platform_GetBmsPowerLimitW(void) { return 6600.0f; }
float CCCP_Platform_GetGridVoltageRmsV(void) { return 220.0f; }
float CCCP_Platform_GetGridPowerFactor(void) { return 0.99f; }
float CCCP_Platform_GetThermalCurrentLimitA(void) { return 30.0f; }
float CCCP_Platform_GetOemCurrentLimitA(void) { return 30.0f; }
float CCCP_Platform_GetOemPowerLimitW(void) { return 6600.0f; }
