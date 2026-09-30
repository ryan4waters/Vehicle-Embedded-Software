#include "Tc377_Hsm_Hal.h"

void Tc377_HsmHal_Init(void)
{
    /*
     * Integrate the exact TC377 HSM firmware/API here.
     *
     * Do not invent/register-map HSM commands in upper layers.
     * Keep mailbox/shared-memory/interrupt details here.
     */
}

Hsm_ReturnType Tc377_HsmHal_Execute(
    Hsm_JobType job,
    Hsm_JobRequestType *req)
{
    if (!req) return HSM_E_PARAM;

    /*
     * Production:
     * 1. Validate job and key policy.
     * 2. Build HSM request.
     * 3. Put non-secret buffers into approved shared memory.
     * 4. Trigger HSM.
     * 5. Receive result asynchronously or synchronously.
     * 6. Validate result and copy only permitted output.
     */
    (void)job;
    return HSM_E_NOT_READY;
}
