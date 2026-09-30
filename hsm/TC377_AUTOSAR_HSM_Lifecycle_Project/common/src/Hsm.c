#include "Hsm.h"
#include "Hsm_Ipc.h"

Hsm_ReturnType Hsm_Init(void)
{
    return Hsm_IpcInit();
}

Hsm_ReturnType Hsm_IsReady(Hsm_U8 *ready)
{
    if (ready == 0) return HSM_E_PARAM;
    *ready = 0u;
    /* Replace by TC377 HSM ready query. */
    return HSM_E_NOT_READY;
}

Hsm_ReturnType Hsm_Submit(
    Hsm_JobType job,
    const Hsm_JobRequestType *req,
    Hsm_JobCallbackType cb,
    Hsm_U32 context)
{
    return Hsm_IpcSubmit(job, req, cb, context);
}

Hsm_ReturnType Hsm_MainFunction(void)
{
    return Hsm_IpcProcess();
}

Hsm_ReturnType Hsm_Sha256(
    const Hsm_U8 *data, Hsm_U32 len, Hsm_U8 digest[32])
{
    Hsm_JobRequestType r = {0};
    r.input = data;
    r.inputLen = len;
    r.output = digest;
    r.outputCapacity = 32u;
    return Hsm_IpcSubmit(HSM_JOB_HASH_SHA256, &r, 0, 0u);
}

Hsm_ReturnType Hsm_CmacGenerate(
    Hsm_U32 keyId, const Hsm_U8 *data, Hsm_U32 len,
    Hsm_U8 *mac, Hsm_U32 *macLen)
{
    Hsm_JobRequestType r = {0};
    if (!macLen) return HSM_E_PARAM;
    r.keyId = keyId;
    r.input = data;
    r.inputLen = len;
    r.output = mac;
    r.outputCapacity = *macLen;
    return Hsm_IpcSubmit(HSM_JOB_CMAC_GENERATE, &r, 0, 0u);
}

Hsm_ReturnType Hsm_CmacVerify(
    Hsm_U32 keyId, const Hsm_U8 *data, Hsm_U32 len,
    const Hsm_U8 *mac, Hsm_U32 macLen)
{
    Hsm_JobRequestType r = {0};
    r.keyId = keyId;
    r.input = data;
    r.inputLen = len;
    r.secondaryInput = mac;
    r.secondaryInputLen = macLen;
    return Hsm_IpcSubmit(HSM_JOB_CMAC_VERIFY, &r, 0, 0u);
}

Hsm_ReturnType Hsm_EcdsaVerify(
    Hsm_U32 publicKeyId, const Hsm_U8 hash[32],
    const Hsm_U8 signature[64])
{
    Hsm_JobRequestType r = {0};
    r.keyId = publicKeyId;
    r.input = hash;
    r.inputLen = 32u;
    r.secondaryInput = signature;
    r.secondaryInputLen = 64u;
    return Hsm_IpcSubmit(HSM_JOB_ECDSA_VERIFY, &r, 0, 0u);
}

Hsm_ReturnType Hsm_Random(Hsm_U8 *out, Hsm_U32 *len)
{
    Hsm_JobRequestType r = {0};
    if (!out || !len) return HSM_E_PARAM;
    r.output = out;
    r.outputCapacity = *len;
    return Hsm_IpcSubmit(HSM_JOB_RANDOM, &r, 0, 0u);
}
