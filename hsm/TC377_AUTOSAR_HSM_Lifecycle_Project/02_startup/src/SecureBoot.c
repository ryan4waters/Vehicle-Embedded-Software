#include "SecureBoot.h"
#include "Hsm.h"
#include "Hsm_Cfg.h"
#include "Tc377_Flash.h"

static Hsm_U32 g_minAllowedVersion = 0u; /* protected persistent counter in production */

SecureBootResultType SecureBoot_Verify(
    const SecureImageInfoType *image)
{
    Hsm_U8 hash[32];

    if (!image || !image->signature)
        return SECBOOT_E_RANGE;

    if (!Tc377_Flash_IsValidRange(image->address, image->size))
        return SECBOOT_E_RANGE;

    if (image->version < g_minAllowedVersion)
        return SECBOOT_E_ROLLBACK;

    if (Hsm_Sha256(
            (const Hsm_U8 *)image->address,
            image->size,
            hash) != HSM_OK)
        return SECBOOT_E_HASH;

    if (Hsm_EcdsaVerify(
            HSM_KEY_BOOT_PUBLIC,
            hash,
            image->signature) != HSM_OK)
        return SECBOOT_E_SIGNATURE;

    return SECBOOT_OK;
}
