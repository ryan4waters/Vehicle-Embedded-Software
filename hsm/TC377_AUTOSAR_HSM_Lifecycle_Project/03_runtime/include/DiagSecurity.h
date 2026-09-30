#ifndef DIAG_SECURITY_H
#define DIAG_SECURITY_H
#include "Hsm_Types.h"

Hsm_ReturnType DiagSecurity_GetSeed(
    Hsm_U8 *seed, Hsm_U32 *seedLen);

Hsm_ReturnType DiagSecurity_VerifyKey(
    const Hsm_U8 *response, Hsm_U32 responseLen);

#endif
