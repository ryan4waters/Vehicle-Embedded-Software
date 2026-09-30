#ifndef HSM_H
#define HSM_H
#include "Hsm_Types.h"

Hsm_ReturnType Hsm_Init(void);
Hsm_ReturnType Hsm_IsReady(Hsm_U8 *ready);
Hsm_ReturnType Hsm_Submit(
    Hsm_JobType job,
    const Hsm_JobRequestType *req,
    Hsm_JobCallbackType cb,
    Hsm_U32 context);
Hsm_ReturnType Hsm_MainFunction(void);
Hsm_ReturnType Hsm_Sha256(
    const Hsm_U8 *data, Hsm_U32 len, Hsm_U8 digest[32]);
Hsm_ReturnType Hsm_CmacGenerate(
    Hsm_U32 keyId, const Hsm_U8 *data, Hsm_U32 len,
    Hsm_U8 *mac, Hsm_U32 *macLen);
Hsm_ReturnType Hsm_CmacVerify(
    Hsm_U32 keyId, const Hsm_U8 *data, Hsm_U32 len,
    const Hsm_U8 *mac, Hsm_U32 macLen);
Hsm_ReturnType Hsm_EcdsaVerify(
    Hsm_U32 publicKeyId, const Hsm_U8 hash[32],
    const Hsm_U8 signature[64]);
Hsm_ReturnType Hsm_Random(
    Hsm_U8 *out, Hsm_U32 *len);

#endif
