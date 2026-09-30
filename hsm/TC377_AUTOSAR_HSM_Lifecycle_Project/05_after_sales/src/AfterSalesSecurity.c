#include "AfterSalesSecurity.h"
#include "SecurityPolicy.h"
#include "Hsm.h"

Hsm_ReturnType AfterSales_AuthenticateTester(
    const Hsm_U8 *request, Hsm_U32 len)
{
    if (!SecurityPolicy_IsAllowed(
            SEC_PHASE_AFTER_SALES, SEC_OP_DIAG_UNLOCK))
        return HSM_E_STATE;

    if (!request || !len)
        return HSM_E_PARAM;

    /*
     * Use project PKI / diagnostic authentication policy.
     */
    return HSM_E_NOT_READY;
}

Hsm_ReturnType AfterSales_StartSecureUpdate(void)
{
    if (!SecurityPolicy_IsAllowed(
            SEC_PHASE_AFTER_SALES, SEC_OP_FIRMWARE_UPDATE))
        return HSM_E_STATE;

    SecurityPolicy_SetPhase(SEC_PHASE_UPDATE);
    return HSM_OK;
}

Hsm_ReturnType AfterSales_RecoverToFactorySafeState(void)
{
    if (!SecurityPolicy_IsAllowed(
            SEC_PHASE_AFTER_SALES, SEC_OP_RECOVERY))
        return HSM_E_STATE;

    /*
     * Recovery must preserve trust anchor / authorized bootloader.
     * Never erase the only valid recovery path.
     */
    return HSM_E_NOT_READY;
}
