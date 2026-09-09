#include "CanStack_Ref.h"
#include "EcuM_Wakeup_Ref.h"

int main(void)
{
    /* Actual TC377 startup is handled by startup code + EcuM. */
    CanStack_Init();

    for (;;) {
        EcuM_CheckWakeupSources();
        CanStack_MainFunction_10ms();

        /*
         * Real AUTOSAR:
         * - OS schedules EcuM_MainFunction
         * - CanSM_MainFunction
         * - CanNm_MainFunction
         * - Com_MainFunction
         * - CanTp_MainFunction
         * - BswM rules
         * etc.
         */
    }

    /* unreachable */
    return 0;
}
