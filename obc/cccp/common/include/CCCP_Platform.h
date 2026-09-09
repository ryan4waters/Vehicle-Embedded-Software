#ifndef CCCP_PLATFORM_H
#define CCCP_PLATFORM_H

#include "CCCP_Types.h"

void CCCP_Platform_Init(void);

/* Analog inputs already converted to physical units. */
float CCCP_Platform_ReadCpVoltage(void);
float CCCP_Platform_ReadCcResistance(void);

/* Hardware capture snapshot. */
CCCP_PlatformPwmCapture CCCP_Platform_GetCpPwmCapture(void);

/* System integration. */
void CCCP_Platform_RequestWakeup(bool enable);
void CCCP_Platform_SetChargePowerEnable(bool enable);

/* External limits. */
float CCCP_Platform_GetBmsCurrentLimitA(void);
float CCCP_Platform_GetBmsPowerLimitW(void);
float CCCP_Platform_GetGridVoltageRmsV(void);
float CCCP_Platform_GetGridPowerFactor(void);
float CCCP_Platform_GetThermalCurrentLimitA(void);
float CCCP_Platform_GetOemCurrentLimitA(void);
float CCCP_Platform_GetOemPowerLimitW(void);

#endif
