#include "PDU_FSM_Example.h"

static FSM_t g_fsm;
static PDU_Context_t g_ctx;

static void Init_Entry(void *ctx) { (void)ctx; }
static void Init_Exit(void *ctx)  { (void)ctx; }

static void Sleep_Entry(void *ctx) { (void)ctx; }
static void Sleep_Exit(void *ctx)  { (void)ctx; }

static void Standby_Entry(void *ctx) { (void)ctx; }
static void Standby_Exit(void *ctx)  { (void)ctx; }

static void Charging_Entry(void *ctx) { (void)ctx; }
static void Charging_Exit(void *ctx)  { (void)ctx; }

static void Charging_Do10ms(void *ctx)
{
    PDU_Context_t *p = (PDU_Context_t *)ctx;
    p->charging_10ms_count++;
}

static void Fault_Entry(void *ctx) { (void)ctx; }
static void Fault_Exit(void *ctx)  { (void)ctx; }

static bool GuardWake(void *ctx)
{
    return ((PDU_Context_t *)ctx)->wake_request;
}

static bool GuardCharge(void *ctx)
{
    return ((PDU_Context_t *)ctx)->charge_request;
}

static bool GuardStop(void *ctx)
{
    return ((PDU_Context_t *)ctx)->stop_request;
}

static bool GuardFault(void *ctx)
{
    return ((PDU_Context_t *)ctx)->fault_active;
}

void PDU_FSM_Init(void)
{
    FSM_Init(&g_fsm, &g_ctx);

    FSM_RegisterState(&g_fsm, PDU_ST_INIT,
                      Init_Entry, Init_Exit, 0U);

    FSM_RegisterState(&g_fsm, PDU_ST_SLEEP,
                      Sleep_Entry, Sleep_Exit, 0U);

    FSM_RegisterState(&g_fsm, PDU_ST_STANDBY,
                      Standby_Entry, Standby_Exit, 0U);

    FSM_RegisterState(&g_fsm, PDU_ST_CHARGING,
                      Charging_Entry, Charging_Exit, 0U);

    FSM_RegisterState(&g_fsm, PDU_ST_FAULT,
                      Fault_Entry, Fault_Exit, 0U);

    FSM_RegisterDoAction(&g_fsm, PDU_ST_CHARGING,
                         Charging_Do10ms, 10U,
                         FSM_ACTION_PERIODIC);

    /* Event is NONE => guard-only transition */
    FSM_RegisterTransition(&g_fsm, PDU_ST_INIT,
                           PDU_ST_SLEEP, FSM_EVENT_NONE,
                           GuardWake, (FSM_TransitionActionFunc)0, 10U);

    FSM_RegisterTransition(&g_fsm, PDU_ST_SLEEP,
                           PDU_ST_STANDBY, FSM_EVENT_NONE,
                           GuardWake, (FSM_TransitionActionFunc)0, 10U);

    FSM_RegisterTransition(&g_fsm, PDU_ST_STANDBY,
                           PDU_ST_FAULT, FSM_EVENT_NONE,
                           GuardFault, (FSM_TransitionActionFunc)0, 0U);

    FSM_RegisterTransition(&g_fsm, PDU_ST_STANDBY,
                           PDU_ST_CHARGING, FSM_EVENT_NONE,
                           GuardCharge, (FSM_TransitionActionFunc)0, 10U);

    FSM_RegisterTransition(&g_fsm, PDU_ST_CHARGING,
                           PDU_ST_FAULT, FSM_EVENT_NONE,
                           GuardFault, (FSM_TransitionActionFunc)0, 0U);

    FSM_RegisterTransition(&g_fsm, PDU_ST_CHARGING,
                           PDU_ST_STANDBY, FSM_EVENT_NONE,
                           GuardStop, (FSM_TransitionActionFunc)0, 10U);

    FSM_RegisterTransition(&g_fsm, PDU_ST_STANDBY,
                           PDU_ST_SLEEP, FSM_EVENT_NONE,
                           GuardStop, (FSM_TransitionActionFunc)0, 20U);

    FSM_RegisterTransition(&g_fsm, PDU_ST_FAULT,
                           PDU_ST_STANDBY, FSM_EVENT_NONE,
                           GuardStop, (FSM_TransitionActionFunc)0, 10U);

    FSM_SetInitialState(&g_fsm, PDU_ST_INIT);
}

void PDU_FSM_1msTask(void)
{
    FSM_Tick(&g_fsm, 1U);
}

void PDU_FSM_10msTask(void)
{
    FSM_Run(&g_fsm);
}

FSM_t *PDU_FSM_Get(void)
{
    return &g_fsm;
}

PDU_Context_t *PDU_GetContext(void)
{
    return &g_ctx;
}
