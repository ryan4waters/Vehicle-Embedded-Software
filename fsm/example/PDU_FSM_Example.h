#ifndef PDU_FSM_EXAMPLE_H
#define PDU_FSM_EXAMPLE_H
#include "FSM_Core.h"

typedef enum {
    PDU_STATE_INIT = 0U,
    PDU_STATE_STANDBY,
    PDU_STATE_CHARGING,
    PDU_STATE_FAULT,
    PDU_STATE_SLEEP
} PDU_StateId_t;

typedef struct {
    bool initDone;
    bool chargeRequest;
    bool chargeStop;
    bool fault;
    bool faultClear;
    bool sleepRequest;
    bool bmsTimeout;
    uint32_t runCounter;
} PDU_FSM_Context_t;

void PDU_FSM_Init(void);
void PDU_FSM_Run(void);
FSM_StateId_t PDU_FSM_GetState(void);

#endif
