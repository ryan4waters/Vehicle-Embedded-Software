#include "UdsUpdate.h"
#include "SecurityPolicy.h"
#include "Tc377_Flash.h"
#include "Hsm.h"
#include "Hsm_Cfg.h"
#include <string.h>

static UdsUpdateStateType g_state = UDS_UPD_IDLE;
static UdsImageMetadataType g_meta;
static Hsm_U32 g_received;

Hsm_ReturnType UdsUpdate_OnSessionControl(Hsm_U8 session)
{
    /* Example: programming session value 0x02. */
    if (session != 0x02u)
        return HSM_E_PARAM;

    SecurityPolicy_SetPhase(SEC_PHASE_UPDATE);
    g_state = UDS_UPD_AUTH;
    return HSM_OK;
}

Hsm_ReturnType UdsUpdate_OnSecurityAccess(
    const Hsm_U8 *request, Hsm_U32 len)
{
    if (!request || !len)
        return HSM_E_PARAM;

    /*
     * Verify authenticated tester/update authorization.
     * Do not expose permanent update secret.
     */
    g_state = UDS_UPD_ERASE;
    return HSM_E_NOT_READY;
}

Hsm_ReturnType UdsUpdate_RequestDownload(
    const UdsImageMetadataType *meta)
{
    if (!meta || !Tc377_Flash_IsValidRange(
            meta->targetAddress, meta->imageSize))
        return HSM_E_PARAM;

    g_meta = *meta;
    g_received = 0u;

    if (Tc377_Flash_Erase(
            meta->targetAddress,
            meta->imageSize) != HSM_OK)
        return HSM_E_NOT_OK;

    g_state = UDS_UPD_DOWNLOAD;
    return HSM_OK;
}

Hsm_ReturnType UdsUpdate_TransferData(
    Hsm_U32 address, const Hsm_U8 *data, Hsm_U32 len)
{
    if (g_state != UDS_UPD_DOWNLOAD)
        return HSM_E_STATE;

    if (!data || (g_received + len > g_meta.imageSize))
        return HSM_E_PARAM;

    Hsm_ReturnType ret =
        Tc377_Flash_Write(address, data, len);

    if (ret == HSM_OK)
        g_received += len;

    return ret;
}

Hsm_ReturnType UdsUpdate_RequestTransferExit(void)
{
    Hsm_U8 hash[32];

    if (g_received != g_meta.imageSize)
        return HSM_E_PARAM;

    g_state = UDS_UPD_HASH;

    if (Hsm_Sha256(
            (const Hsm_U8 *)g_meta.targetAddress,
            g_meta.imageSize,
            hash) != HSM_OK)
        return HSM_E_AUTH;

    if (memcmp(hash, g_meta.expectedHash, 32u) != 0)
        return HSM_E_AUTH;

    g_state = UDS_UPD_SIGNATURE;

    if (Hsm_EcdsaVerify(
            HSM_KEY_UPDATE_AUTH,
            hash,
            g_meta.signature) != HSM_OK)
        return HSM_E_AUTH;

    g_state = UDS_UPD_VERSION;
    /*
     * Anti-rollback check goes here.
     * Persist monotonic counter only after signature/hash checks.
     */

    return HSM_OK;
}

Hsm_ReturnType UdsUpdate_RoutineControlCommit(void)
{
    if (g_state != UDS_UPD_VERSION)
        return HSM_E_STATE;

    /*
     * Atomic commit:
     * mark image valid only after all verification passes.
     * Prefer A/B or staging partition for power-loss resilience.
     */
    g_state = UDS_UPD_COMMIT;
    return HSM_OK;
}
