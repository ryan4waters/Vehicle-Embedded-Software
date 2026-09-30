#ifndef CRYPTO_ADAPTER_H
#define CRYPTO_ADAPTER_H
#include "Hsm_Types.h"

Hsm_ReturnType CryptoAdapter_Hash(
    const Hsm_U8 *data, Hsm_U32 len, Hsm_U8 digest[32]);

Hsm_ReturnType CryptoAdapter_CmacGenerate(
    Hsm_U32 keyId, const Hsm_U8 *data, Hsm_U32 len,
    Hsm_U8 *mac, Hsm_U32 *macLen);

Hsm_ReturnType CryptoAdapter_CmacVerify(
    Hsm_U32 keyId, const Hsm_U8 *data, Hsm_U32 len,
    const Hsm_U8 *mac, Hsm_U32 macLen);

Hsm_ReturnType CryptoAdapter_EcdsaVerify(
    Hsm_U32 publicKeyId, const Hsm_U8 hash[32],
    const Hsm_U8 signature[64]);

#endif
