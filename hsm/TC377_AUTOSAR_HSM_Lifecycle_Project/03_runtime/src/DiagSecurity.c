#include "DiagSecurity.h"
#include "Hsm.h"
#include "SecurityPolicy.h"

Hsm_ReturnType DiagSecurity_GetSeed(
    Hsm_U8 *seed, Hsm_U32 *seedLen)
{
    if (!SecurityPolicy_IsAllowed(
            SEC_PHASE_RUNTIME, SEC_OP_DIAG_UNLOCK))
        return HSM_E_STATE;

    return Hsm_Random(seed, seedLen);
}

Hsm_ReturnType DiagSecurity_VerifyKey(
    const Hsm_U8 *response, Hsm_U32 responseLen)
{
    /*
     * The actual UDS 0x27 algorithm should be executed by HSM/CSM
     * without exposing permanent secrets to the diagnostic application.
     */
    if (!response || !responseLen)
        return HSM_E_PARAM;

    return HSM_E_NOT_READY;
}
