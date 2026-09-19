#include "PDU_FSM_Example.h"

static PDU_FSM_Context_t g_ctx;
static FSM_t g_fsm;

static void Entry_Init(void *c) { (void)c; }
static void Exit_Init(void *c)  { (void)c; }
static void Do_Init(void *c)    { ((PDU_FSM_Context_t *)c)->runCounter++; }

static void Entry_Standby(void *c) { (void)c; }
static void Exit_Standby(void *c)  { (void)c; }
static void Do_Standby(void *c)    { ((PDU_FSM_Context_t *)c)->runCounter++; }

static void Entry_Charging(void *c) { (void)c; }
static void Exit_Charging(void *c)  { (void)c; }
static void Do_Charging(void *c)    { ((PDU_FSM_Context_t *)c)->runCounter++; }

static void Entry_Fault(void *c) { (void)c; }
static void Exit_Fault(void *c)  { (void)c; }
static void Do_Fault(void *c)    { (void)c; }

static void Entry_Sleep(void *c) { (void)c; }
static void Exit_Sleep(void *c)  { (void)c; }
static void Do_Sleep(void *c)    { (void)c; }

static const FSM_DoAction_t g_initDo[] = {
    { Do_Init, 1U }
};
static const FSM_DoAction_t g_standbyDo[] = {
    { Do_Standby, 10U }
};
static const FSM_DoAction_t g_chargingDo[] = {
    { Do_Charging, 1U }
};
static const FSM_DoAction_t g_faultDo[] = {
    { Do_Fault, 10U }
};
static const FSM_DoAction_t g_sleepDo[] = {
    { Do_Sleep, 100U }
};

static const FSM_State_t g_states[] = {
    { PDU_STATE_INIT,     Entry_Init,     Exit_Init,     g_initDo,     1U },
    { PDU_STATE_STANDBY,  Entry_Standby,  Exit_Standby,  g_standbyDo,  1U },
    { PDU_STATE_CHARGING, Entry_Charging, Exit_Charging, g_chargingDo, 1U },
    { PDU_STATE_FAULT,    Entry_Fault,    Exit_Fault,    g_faultDo,    1U },
    { PDU_STATE_SLEEP,    Entry_Sleep,    Exit_Sleep,    g_sleepDo,    1U }
};

/* Conditions contain decisions only; side effects belong to Entry/Exit/Do. */
static bool Cond_InitDone(void *c)     { return ((PDU_FSM_Context_t *)c)->initDone; }
static bool Cond_Fault(void *c)        { return ((PDU_FSM_Context_t *)c)->fault; }
static bool Cond_ChargeRequest(void *c){ return ((PDU_FSM_Context_t *)c)->chargeRequest; }
static bool Cond_SleepRequest(void *c) { return ((PDU_FSM_Context_t *)c)->sleepRequest; }
static bool Cond_ChargeStop(void *c)   { return ((PDU_FSM_Context_t *)c)->chargeStop; }
static bool Cond_FaultClear(void *c)   { return ((PDU_FSM_Context_t *)c)->faultClear; }
static bool Cond_BmsTimeout(void *c)   { return ((PDU_FSM_Context_t *)c)->bmsTimeout; }

/*
 * Global transition table.
 * Order is priority for transitions from the same current state.
 */
static const FSM_Transition_t g_transitions[] = {
    { PDU_STATE_INIT,     PDU_STATE_STANDBY,  Cond_InitDone },

    { PDU_STATE_STANDBY,  PDU_STATE_FAULT,    Cond_Fault },
    { PDU_STATE_STANDBY,  PDU_STATE_CHARGING, Cond_ChargeRequest },
    { PDU_STATE_STANDBY,  PDU_STATE_SLEEP,    Cond_SleepRequest },

    { PDU_STATE_CHARGING, PDU_STATE_FAULT,    Cond_Fault },
    { PDU_STATE_CHARGING, PDU_STATE_STANDBY,  Cond_ChargeStop },
    { PDU_STATE_CHARGING, PDU_STATE_FAULT,    Cond_BmsTimeout },

    { PDU_STATE_FAULT,    PDU_STATE_STANDBY,  Cond_FaultClear }
};

void PDU_FSM_Init(void)
{
    g_ctx = (PDU_FSM_Context_t){0};

    (void)FSM_Init(&g_fsm,
                   g_states, (uint16_t)(sizeof(g_states)/sizeof(g_states[0])),
                   g_transitions, (uint16_t)(sizeof(g_transitions)/sizeof(g_transitions[0])),
                   &g_ctx);

    (void)FSM_SetInitialState(&g_fsm, PDU_STATE_INIT);
}

void PDU_FSM_Run(void)
{
    FSM_Run(&g_fsm);
}

FSM_StateId_t PDU_FSM_GetState(void)
{
    return FSM_GetCurrentStateId(&g_fsm);
}
