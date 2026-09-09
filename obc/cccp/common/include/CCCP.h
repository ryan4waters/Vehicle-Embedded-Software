#ifndef CCCP_H
#define CCCP_H

#include "CCCP_Types.h"

void CCCP_Init(void);
void CCCP_1msTask(void);
void CCCP_10msTask(void);

const CCCP_Status *CCCP_GetStatus(void);
CCCP_State CCCP_GetState(void);

bool CCCP_IsPlugged(void);
bool CCCP_IsVehicleReady(void);
bool CCCP_IsChargeAllowed(void);

#endif
