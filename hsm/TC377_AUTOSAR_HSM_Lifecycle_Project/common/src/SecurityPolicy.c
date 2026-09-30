#include "SecurityPolicy.h"

static SecurityPhaseType g_phase = SEC_PHASE_STARTUP;

void SecurityPolicy_SetPhase(SecurityPhaseType phase)
{
    g_phase = phase;
}

SecurityPhaseType SecurityPolicy_GetPhase(void)
{
    return g_phase;
}

Hsm_U8 SecurityPolicy_IsAllowed(
    SecurityPhaseType phase,
    SecurityOperationType operation)
{
    switch (phase)
    {
        case SEC_PHASE_FACTORY:
            return (operation == SEC_OP_KEY_PROVISION ||
                    operation == SEC_OP_KEY_ROTATE ||
                    operation == SEC_OP_BOOT_VERIFY);

        case SEC_PHASE_STARTUP:
            return (operation == SEC_OP_BOOT_VERIFY);

        case SEC_PHASE_RUNTIME:
            return (operation == SEC_OP_SECOC ||
                    operation == SEC_OP_DIAG_UNLOCK);

        case SEC_PHASE_UPDATE:
            return (operation == SEC_OP_FIRMWARE_UPDATE ||
                    operation == SEC_OP_BOOT_VERIFY);

        case SEC_PHASE_AFTER_SALES:
            return (operation == SEC_OP_DIAG_UNLOCK ||
                    operation == SEC_OP_FIRMWARE_UPDATE ||
                    operation == SEC_OP_RECOVERY);

        default:
            return 0u;
    }
}
