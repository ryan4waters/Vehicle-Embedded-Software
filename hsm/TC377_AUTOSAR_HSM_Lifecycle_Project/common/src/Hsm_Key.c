#include "Hsm_Key.h"
#include "SecurityPolicy.h"

Hsm_ReturnType Hsm_KeyGetState(
    Hsm_U32 keyId,
    Hsm_KeyStateType *state)
{
    if (!state) return HSM_E_PARAM;
    (void)keyId;
    *state = HSM_KEY_VALID; /* placeholder: query HSM */
    return HSM_OK;
}

Hsm_ReturnType Hsm_KeySetValid(Hsm_U32 keyId)
{
    if (!SecurityPolicy_IsAllowed(
            SEC_PHASE_FACTORY, SEC_OP_KEY_PROVISION))
        return HSM_E_STATE;
    (void)keyId;
    return HSM_E_NOT_READY;
}

Hsm_ReturnType Hsm_KeyInvalidate(Hsm_U32 keyId)
{
    (void)keyId;
    return HSM_E_NOT_READY;
}
