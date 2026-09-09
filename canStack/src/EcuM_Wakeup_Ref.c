#include "EcuM_Wakeup_Ref.h"
#include "Tja1145_Drv.h"

static bool Kl15_IsOn(void)
{
    /* Replace with Port/Dio/MCU wake input implementation. */
    return false;
}

void EcuM_CheckWakeupSources(void)
{
    if (Kl15_IsOn()) {
        EcuM_ProcessWakeup(CAN_WAKE_KL15);
        return;
    }

    Tja1145_WakeupSourceType src = Tja1145_GetWakeupSource();

    switch (src) {
    case TJA1145_WAKE_CAN:
        EcuM_ProcessWakeup(CAN_WAKE_CAN_BUS);
        break;

    case TJA1145_WAKE_SELECTIVE:
        EcuM_ProcessWakeup(CAN_WAKE_NM_SELECTIVE);
        break;

    case TJA1145_WAKE_LOCAL:
        EcuM_ProcessWakeup(CAN_WAKE_KL15);
        break;

    default:
        break;
    }
}

void EcuM_ProcessWakeup(CanStack_WakeupSourceType source)
{
    /*
     * Real AUTOSAR sequence is conceptually:
     *
     * EcuM_SetWakeupEvent()
     * -> EcuM_CheckWakeup()
     * -> EcuM_ValidateWakeupEvent()
     * -> ComM/CanSM communication request
     *
     * Exact APIs depend on AUTOSAR release/vendor implementation.
     */
    CanStack_HandleWakeup(source);
}
