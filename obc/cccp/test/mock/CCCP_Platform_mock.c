#include "CCCP_Platform.h"

static CCCP_PlatformPwmCapture g_cap = {
    1000U, 500U, 1000000U, true
};

void CCCP_Platform_Init(void) {}
float CCCP_Platform_ReadCpVoltage(void) { return 6.0f; }
float CCCP_Platform_ReadCcResistance(void) { return 220.0f; }
CCCP_PlatformPwmCapture CCCP_Platform_GetCpPwmCapture(void) { return g_cap; }
void CCCP_Platform_RequestWakeup(bool enable) { (void)enable; }
void CCCP_Platform_SetChargePowerEnable(bool enable) { (void)enable; }
float CCCP_Platform_GetBmsCurrentLimitA(void) { return 25.0f; }
float CCCP_Platform_GetBmsPowerLimitW(void) { return 5500.0f; }
float CCCP_Platform_GetGridVoltageRmsV(void) { return 220.0f; }
float CCCP_Platform_GetGridPowerFactor(void) { return 0.99f; }
float CCCP_Platform_GetThermalCurrentLimitA(void) { return 24.0f; }
float CCCP_Platform_GetOemCurrentLimitA(void) { return 30.0f; }
float CCCP_Platform_GetOemPowerLimitW(void) { return 6600.0f; }
