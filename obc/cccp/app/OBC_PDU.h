#ifndef OBC_PDU_H
#define OBC_PDU_H

#include "CCCP_Types.h"

typedef enum
{
    PDU_OFF = 0,
    PDU_WAIT_PLUG,
    PDU_PLUGGED,
    PDU_WAIT_BMS,
    PDU_PRECHARGE,
    PDU_CHARGING,
    PDU_STOPPING,
    PDU_FAULT
} OBC_PDU_State;

void OBC_PDU_Init(void);
void OBC_PDU_10msTask(const CCCP_Status *cccp);
OBC_PDU_State OBC_PDU_GetState(void);

#endif
