#include "SecOC_Runtime.h"
#include "Hsm.h"
#include "Hsm_Cfg.h"
#include "SecurityPolicy.h"
#include <string.h>

static Hsm_U32 BuildInput(
    const SecCanFrameType *frame,
    Hsm_U32 freshness,
    Hsm_U8 *buf,
    Hsm_U32 cap)
{
    if (!frame || !buf || cap < (8u + frame->dlc))
        return 0u;

    buf[0] = (Hsm_U8)(frame->canId >> 24);
    buf[1] = (Hsm_U8)(frame->canId >> 16);
    buf[2] = (Hsm_U8)(frame->canId >> 8);
    buf[3] = (Hsm_U8)frame->canId;

    buf[4] = (Hsm_U8)(freshness >> 24);
    buf[5] = (Hsm_U8)(freshness >> 16);
    buf[6] = (Hsm_U8)(freshness >> 8);
    buf[7] = (Hsm_U8)freshness;

    memcpy(&buf[8], frame->data, frame->dlc);
    return 8u + frame->dlc;
}

Hsm_ReturnType SecOC_RuntimeTx(
    const SecCanFrameType *frame,
    Hsm_U32 freshness,
    Hsm_U8 *mac,
    Hsm_U32 *macLen)
{
    Hsm_U8 input[16];
    Hsm_U32 len;

    if (!SecurityPolicy_IsAllowed(
            SEC_PHASE_RUNTIME, SEC_OP_SECOC))
        return HSM_E_STATE;

    len = BuildInput(frame, freshness, input, sizeof(input));
    if (!len) return HSM_E_PARAM;

    return Hsm_CmacGenerate(
        HSM_KEY_SECOC_CMAC,
        input,
        len,
        mac,
        macLen);
}

Hsm_ReturnType SecOC_RuntimeRxVerify(
    const SecCanFrameType *frame,
    Hsm_U32 freshness,
    const Hsm_U8 *mac,
    Hsm_U32 macLen)
{
    Hsm_U8 input[16];
    Hsm_U32 len;

    if (!SecurityPolicy_IsAllowed(
            SEC_PHASE_RUNTIME, SEC_OP_SECOC))
        return HSM_E_STATE;

    len = BuildInput(frame, freshness, input, sizeof(input));
    if (!len) return HSM_E_PARAM;

    return Hsm_CmacVerify(
        HSM_KEY_SECOC_CMAC,
        input,
        len,
        mac,
        macLen);
}
