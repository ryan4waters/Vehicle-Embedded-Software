#include "FactorySecurity.h"
#include "SecurityPolicy.h"
#include "Hsm.h"
#include "Hsm_Cfg.h"

Hsm_ReturnType FactorySecurity_ProvisionDevice(
    const FactoryDeviceIdentityType *identity)
{
    if (!identity || !identity->certificate)
        return HSM_E_PARAM;

    if (!SecurityPolicy_IsAllowed(
            SEC_PHASE_FACTORY, SEC_OP_KEY_PROVISION))
        return HSM_E_STATE;

    /*
     * The actual certificate/private key provisioning should use
     * the authorized manufacturing flow. Never compile real keys.
     */
    (void)identity;
    return HSM_E_NOT_READY;
}

Hsm_ReturnType FactorySecurity_RunEndOfLineSecurityTest(void)
{
    Hsm_U8 ready = 0u;

    if (Hsm_IsReady(&ready) != HSM_OK || !ready)
        return HSM_E_NOT_READY;

    /*
     * Add:
     * - HSM self test
     * - boot signature test
     * - SecOC MAC test
     * - RNG test
     * - diagnostic authentication test
     */
    return HSM_OK;
}

Hsm_ReturnType FactorySecurity_ProductionLockPreCheck(void)
{
    /*
     * Before production lock:
     * - recovery path verified
     * - correct HSM firmware
     * - correct boot image
     * - key states verified
     * - debug policy verified
     * - UCB image reviewed
     */
    return HSM_OK;
}
