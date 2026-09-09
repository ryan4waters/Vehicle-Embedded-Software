#include "CanStack_Ref.h"
#include "Tja1145_Drv.h"
#include "Tc377_CanHw.h"

static CanStack_StateType g_state = CANSTACK_NO_COMM;
static CanStack_WakeupSourceType g_wakeup = CAN_WAKE_NONE;

static void CanStack_StartCommunication(void)
{
    /* Real AUTOSAR project:
     * EcuM/BswM/ComM/CanSM coordinate these operations.
     * This reference implementation only shows the sequence.
     */
    Tc377_Can_SetControllerMode(true);
    Tja1145_SetNormalMode();
    g_state = CANSTACK_FULL_COMM;
}

static void CanStack_StopCommunication(void)
{
    /* Real project normally reaches here through:
     * ComM -> CanSM -> CanIf -> CanDrv
     */
    Tc377_Can_SetControllerMode(false);
    Tja1145_SetSleepMode();
    g_state = CANSTACK_NO_COMM;
}

void CanStack_Init(void)
{
    Tc377_Can_Init();
    Tja1145_Init();
    g_state = CANSTACK_NO_COMM;
    g_wakeup = CAN_WAKE_NONE;
}

void CanStack_HandleWakeup(CanStack_WakeupSourceType source)
{
    g_wakeup = source;
    g_state = CANSTACK_WAKEUP;

    /* EcuM wakeup validation would normally happen before
     * communication is requested. */
    CanStack_StartCommunication();
}

void CanStack_RequestFullCommunication(void)
{
    if (g_state == CANSTACK_NO_COMM ||
        g_state == CANSTACK_WAKEUP ||
        g_state == CANSTACK_BUSOFF_RECOVERY) {
        CanStack_StartCommunication();
    }
}

void CanStack_RequestNoCommunication(void)
{
    if (g_state == CANSTACK_FULL_COMM) {
        g_state = CANSTACK_PREPARE_SLEEP;
    }
}

void CanStack_HandleBusOff(void)
{
    g_state = CANSTACK_BUSOFF_RECOVERY;
    Tc377_Can_SetControllerMode(false);
    /* Recovery timer / CanSM logic belongs here in a simplified model. */
    Tc377_Can_SetControllerMode(true);
    g_state = CANSTACK_FULL_COMM;
}

void CanStack_MainFunction_10ms(void)
{
    /* In a real AUTOSAR stack this function is distributed across
     * ComM/CanSM/CanNm/BswM/EcuM main functions. */
    if (g_state == CANSTACK_PREPARE_SLEEP) {
        CanStack_StopCommunication();
    }
}

CanStack_StateType CanStack_GetState(void)
{
    return g_state;
}

CanStack_WakeupSourceType CanStack_GetWakeupSource(void)
{
    return g_wakeup;
}
