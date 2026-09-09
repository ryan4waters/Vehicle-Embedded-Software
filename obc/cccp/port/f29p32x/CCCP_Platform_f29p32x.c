#include "CCCP_Platform_f29p32x.h"

/*
 * TI F29P32x adapter.
 *
 * Keep C2000Ware DriverLib calls here.
 * Do not expose DriverLib types to common/.
 */

static volatile uint32_t g_cp_period_ticks;
static volatile uint32_t g_cp_high_ticks;
static volatile bool g_cp_capture_valid;

static const uint32_t g_cp_capture_timer_hz = 1000000U;

void CCCP_Platform_Init(void)
{
    /*
     * TODO:
     *
     * Device_init();
     * Device_initGPIO();
     *
     * Configure ADC SOC for CP/CC.
     * Configure eCAP/capture peripheral for rising/falling timestamps.
     * Configure GPIO/XINT if used for wakeup.
     *
     * Exact API names and pin mapping must match the installed F29P32x
     * C2000Ware release and device header.
     */
}

float CCCP_Platform_ReadCpVoltage(void)
{
    /* TODO: ADC result -> CP physical voltage. */
    return 12.0f;
}

float CCCP_Platform_ReadCcResistance(void)
{
    /* TODO: CC analog front-end -> cable resistor. */
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
