#ifndef HSM_KEY_H
#define HSM_KEY_H
#include "Hsm_Types.h"

typedef enum {
    HSM_KEY_EMPTY = 0,
    HSM_KEY_PROVISIONED,
    HSM_KEY_VALID,
    HSM_KEY_RETIRED,
    HSM_KEY_DESTROYED
} Hsm_KeyStateType;

typedef struct {
    Hsm_U32 keyId;
    Hsm_KeyStateType state;
} Hsm_KeyInfoType;

Hsm_ReturnType Hsm_KeyGetState(
    Hsm_U32 keyId,
    Hsm_KeyStateType *state);

Hsm_ReturnType Hsm_KeySetValid(Hsm_U32 keyId);
Hsm_ReturnType Hsm_KeyInvalidate(Hsm_U32 keyId);

#endif
