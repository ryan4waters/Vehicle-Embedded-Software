#ifndef PDU_FSM_EXAMPLE_H
#define PDU_FSM_EXAMPLE_H

#include "FSM_Core.h"

typedef enum
{
    PDU_ST_INIT = 0U,
    PDU_ST_SLEEP,
    PDU_ST_STANDBY,
    PDU_ST_CHARGING,
    PDU_ST_FAULT
} PDU_StateId_t;

typedef struct
{
    bool wake_request;
    bool charge_request;
    bool stop_request;
    bool fault_active;
    uint32_t charging_10ms_count;
} PDU_Context_t;

void PDU_FSM_Init(void);
void PDU_FSM_1msTask(void);
void PDU_FSM_10msTask(void);
FSM_t *PDU_FSM_Get(void);
PDU_Context_t *PDU_GetContext(void);

#endif
