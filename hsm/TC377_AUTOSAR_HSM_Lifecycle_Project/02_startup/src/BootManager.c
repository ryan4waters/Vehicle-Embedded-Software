#include "BootManager.h"
#include "Hsm.h"
#include "SecureBoot.h"
#include "SecurityPolicy.h"

static BootStateType g_state = BOOT_INIT;

void BootManager_Init(void)
{
    SecurityPolicy_SetPhase(SEC_PHASE_STARTUP);
    (void)Hsm_Init();
    g_state = BOOT_HSM_WAIT;
}

void BootManager_MainFunction(void)
{
    Hsm_U8 ready = 0u;
    SecureBootResultType ret;

    switch (g_state)
    {
        case BOOT_HSM_WAIT:
            if (Hsm_IsReady(&ready) == HSM_OK && ready)
                g_state = BOOT_VERIFY;
            break;

        case BOOT_VERIFY:
        {
            /*
             * Replace with linker/generated image metadata.
             */
            SecureImageInfoType image = {
                0x800A0000u, 0u, 0u, 0
            };

            ret = SecureBoot_Verify(&image);
            g_state = (ret == SECBOOT_OK)
                    ? BOOT_START_APP
                    : BOOT_RECOVERY;
            break;
        }

        case BOOT_START_APP:
            /* Validate vector/metadata and jump to application. */
            break;

        case BOOT_RECOVERY:
            /* Remain in recovery/update mode. */
            break;

        default:
            break;
    }
}
