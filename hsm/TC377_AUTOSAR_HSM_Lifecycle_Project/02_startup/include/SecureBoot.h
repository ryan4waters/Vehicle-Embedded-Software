#ifndef SECURE_BOOT_H
#define SECURE_BOOT_H
#include "Hsm_Types.h"

typedef struct {
    Hsm_U32 address;
    Hsm_U32 size;
    Hsm_U32 version;
    const Hsm_U8 *signature;
} SecureImageInfoType;

typedef enum {
    SECBOOT_OK = 0,
    SECBOOT_E_HSM,
    SECBOOT_E_RANGE,
    SECBOOT_E_HASH,
    SECBOOT_E_SIGNATURE,
    SECBOOT_E_ROLLBACK
} SecureBootResultType;

SecureBootResultType SecureBoot_Verify(
    const SecureImageInfoType *image);

#endif
