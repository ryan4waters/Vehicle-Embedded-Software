#ifndef HSM_TYPES_H
#define HSM_TYPES_H
#include <stdint.h>

typedef uint8_t  Hsm_U8;
typedef uint16_t Hsm_U16;
typedef uint32_t Hsm_U32;
typedef uint64_t Hsm_U64;

typedef enum {
    HSM_OK = 0,
    HSM_E_NOT_OK,
    HSM_E_BUSY,
    HSM_E_TIMEOUT,
    HSM_E_PARAM,
    HSM_E_AUTH,
    HSM_E_ROLLBACK,
    HSM_E_NOT_READY,
    HSM_E_STATE
} Hsm_ReturnType;

typedef enum {
    HSM_JOB_HASH_SHA256 = 0,
    HSM_JOB_CMAC_GENERATE,
    HSM_JOB_CMAC_VERIFY,
    HSM_JOB_ECDSA_VERIFY,
    HSM_JOB_RANDOM,
    HSM_JOB_KEY_PROVISION,
    HSM_JOB_KEY_ROTATE,
    HSM_JOB_KEY_INVALIDATE
} Hsm_JobType;

typedef struct {
    Hsm_U32 keyId;
    const Hsm_U8 *input;
    Hsm_U32 inputLen;
    const Hsm_U8 *secondaryInput;
    Hsm_U32 secondaryInputLen;
    Hsm_U8 *output;
    Hsm_U32 outputCapacity;
    Hsm_U32 outputLen;
} Hsm_JobRequestType;

typedef void (*Hsm_JobCallbackType)(
    Hsm_JobType job,
    Hsm_ReturnType result,
    Hsm_U32 context);

#endif
